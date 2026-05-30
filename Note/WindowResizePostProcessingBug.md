# Window Resize + Post Processing Bug

작성일: 2026-05-30  
범위: 윈도우 크기 변경 시 발생한 렌더링/후처리 버그 원인과 해결 흐름 정리

---

## 최종 결론

윈도우 리사이즈 버그의 핵심 원인은 **swap chain back buffer와 연결된 리소스가 완전히 정리되지 않은 상태에서 `ResizeBuffers()`를 호출한 것**이었다.

특히 post-processing을 켠 상태에서는 `m_shaderResourceView`, `m_tempTexture`, `ImageFilter` 체인이 back buffer 또는 back buffer에서 파생된 리소스와 연결되어 있었고, 이 참조가 남아 `ResizeBuffers()`가 실패했다.

최종 해결 방향:

```text
ResizeBuffers() 전
  - back buffer 관련 D3D 바인딩 해제
  - RTV/SRV/depth/tempTexture Reset

ResizeBuffers() 후
  - 새 back buffer 기준 RTV 재생성
  - 새 tempTexture/SRV 재생성
  - depth buffer 재생성
  - viewport 갱신
  - camera aspect 갱신
  - post-processing filter chain 재생성
```

---

## 발생 증상

| 상황 | 증상 |
|---|---|
| Post Processing OFF 상태에서 리사이즈 | 초기에는 검정 화면, 비율 이상, 렌더링 범위 이상 등이 발생 |
| Post Processing ON 상태에서 리사이즈 | 크래시 또는 화면 일부 검정, 구 일부 검정, 렌더링 영역 축소 |
| `ResizeBuffers()` 반환값 확인 후 | `ResizeBuffers() failed. 887a0001` 반복 출력 |
| 최종 수정 후 | Post Processing ON/OFF 모두 리사이즈 정상 동작 |

`0x887a0001`은 이 상황에서 `DXGI_ERROR_INVALID_CALL`로 볼 수 있고, 보통 swap chain buffer를 resize하기 전에 관련 참조가 아직 남아 있을 때 발생한다.

---

## 원인 분석

### 1. `ResizeBuffers()` 전에 back buffer 관련 참조가 남아 있었음

윈도우 크기가 바뀌면 swap chain의 back buffer 크기도 바뀌어야 한다.

하지만 기존 코드에서는 render target view만 reset하고 `ResizeBuffers()`를 호출했다.

```text
m_renderTargetView.Reset()
ResizeBuffers()
CreateRenderTargetView()
```

이 흐름만으로는 부족했다.

post-processing 사용 시 다음 리소스들이 back buffer 또는 back buffer에서 파생된 리소스를 참조할 수 있다.

| 리소스 | 역할 |
|---|---|
| `m_renderTargetView` | swap chain back buffer에 렌더링하기 위한 RTV |
| `m_shaderResourceView` | 후처리 입력용 SRV |
| `m_tempTexture` | MSAA back buffer를 resolve해서 후처리 입력으로 쓰는 텍스처 |
| `ImageFilter` 내부 SRV/RTV | 후처리 필터 체인의 입력/출력 리소스 |

따라서 `ResizeBuffers()` 전에 RTV뿐 아니라 관련 SRV, depth, temp texture, pipeline binding을 정리해야 했다.

---

### 2. Back buffer에 대한 SRV를 불필요하게 생성하고 있었음

기존 `CreateRenderTargetView()`에서는 back buffer로 RTV를 만든 뒤, back buffer 자체에 대한 SRV도 만들고 있었다.

```text
backBuffer -> RTV 생성
backBuffer -> SRV 생성
tempTexture -> SRV 생성
```

하지만 실제 후처리 구조에서는 MSAA back buffer를 직접 SRV로 읽지 않고, `ResolveSubresource()`로 `m_tempTexture`에 복사한 뒤 그 texture의 SRV를 읽는다.

실제 사용 흐름:

```text
backBuffer
  -> ResolveSubresource()
m_tempTexture
  -> m_shaderResourceView
ImageFilter input
```

따라서 back buffer 자체의 SRV는 필요하지 않았다.

수정 후 흐름:

```text
backBuffer -> RTV 생성
tempTexture -> SRV 생성
```

이렇게 바꾸면서 back buffer 참조가 불필요하게 남는 문제를 제거했다.

---

### 3. 후처리 필터는 화면 크기에 의존하므로 리사이즈 후 재생성이 필요함

`ImageFilter`는 생성 시 width/height를 기준으로 내부 render target texture와 viewport를 만든다.

```text
ImageFilter(width, height)
  - 내부 texture 생성
  - RTV/SRV 생성
  - viewport 설정
```

따라서 윈도우 크기가 바뀌면 기존 필터 체인은 이전 크기의 텍스처와 viewport를 계속 사용하게 된다.

해결:

```text
AppBase::OnResize()
  -> Ryudar::OnResize()
  -> BuildFilters()
```

리사이즈 후 새 화면 크기에 맞춰 후처리 필터 체인을 다시 만든다.

---

### 4. Viewport state는 후처리 패스에서 바뀔 수 있음

후처리 필터는 자기 크기에 맞는 viewport를 렌더링 중에 설정한다.

```text
ImageFilter::Render()
  -> RSSetViewports(filterViewport)
```

그래서 다음 메인 렌더링 시작 시 screen viewport를 다시 바인딩해야 한다.

해결 방향:

```text
SetViewport()
  - 화면 크기나 GUI 폭이 바뀌면 viewport 값 재계산
  - D3D context에는 매번 screen viewport를 다시 바인딩
```

이렇게 하면 post-processing이 viewport state를 변경해도 다음 프레임의 메인 렌더링은 정상 screen viewport에서 시작한다.

---

## 수정된 리사이즈 처리 흐름

`WM_SIZE` 처리의 최종 흐름:

```text
1. 새 width/height 저장
2. width/height가 0이면 중단
3. back buffer 관련 pipeline binding 해제
4. 기존 RTV/SRV/depth/tempTexture Reset
5. ResizeBuffers() 호출
6. 실패 시 로그 출력 후 중단
7. 새 back buffer 기준 RTV 생성
8. 새 tempTexture/SRV 생성
9. depth buffer 재생성
10. viewport 갱신 및 바인딩
11. camera aspect 갱신
12. OnResize() 호출
13. Ryudar에서 post-processing filter chain 재생성
```

---

## 핵심 코드 흐름

### `AppBase::MsgProc(WM_SIZE)`

```cpp
m_screenWidth = int(LOWORD(lParam));
m_screenHeight = int(HIWORD(lParam));

if (m_screenWidth == 0 || m_screenHeight == 0)
{
    break;
}

m_context->OMSetRenderTargets(0, nullptr, nullptr);

ID3D11ShaderResourceView *nullSRV[16] = {};
m_context->PSSetShaderResources(0, 16, nullSRV);

m_renderTargetView.Reset();
m_shaderResourceView.Reset();
m_depthStencilView.Reset();
m_tempTexture.Reset();

HRESULT hr = m_swapChain->ResizeBuffers(
    0,
    UINT(m_screenWidth),
    UINT(m_screenHeight),
    DXGI_FORMAT_UNKNOWN,
    0);

if (FAILED(hr))
{
    std::cout << "ResizeBuffers() failed. " << std::hex << hr << std::endl;
    break;
}

CreateRenderTargetView();
D3D11Utils::CreateDepthBuffer(
    m_device,
    m_screenWidth,
    m_screenHeight,
    numQualityLevels,
    m_depthStencilView);

SetViewport();
m_camera.SetAspectRatio(GetAspectRatio());
OnResize();
```

---

### `AppBase::CreateRenderTargetView()`

중요한 점:

| 이전 | 수정 후 |
|---|---|
| back buffer로 RTV 생성 | 유지 |
| back buffer로 SRV 생성 | 제거 |
| tempTexture로 SRV 생성 | 유지 |

수정 후 의도:

```text
swap chain back buffer는 렌더링 대상
tempTexture는 후처리 입력 대상
```

```cpp
m_renderTargetView.Reset();
m_shaderResourceView.Reset();
m_tempTexture.Reset();

ComPtr<ID3D11Texture2D> backBuffer;
m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));

if (backBuffer)
{
    m_device->CreateRenderTargetView(
        backBuffer.Get(),
        NULL,
        m_renderTargetView.GetAddressOf());

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = 0;

    if (FAILED(m_device->CreateTexture2D(&desc, nullptr, m_tempTexture.GetAddressOf())))
    {
        cout << "Create temp texture failed." << endl;
    }

    m_device->CreateShaderResourceView(
        m_tempTexture.Get(),
        nullptr,
        m_shaderResourceView.GetAddressOf());
}
```

---

### `Ryudar::OnResize()`

```cpp
void Ryudar::OnResize()
{
    BuildFilters();
    m_dirtyflag = 1;
}
```

의도:

```text
윈도우 크기 변경
  -> AppBase 공통 D3D 리소스 재생성
  -> Ryudar 전용 후처리 리소스 재생성
  -> 다음 Update에서 bloom threshold/strength 반영
```

---

## Post Processing 렌더링 흐름

현재 후처리 렌더링 흐름:

```text
1. Main scene render
   backBuffer RTV에 scene 렌더링

2. Resolve
   backBuffer -> m_tempTexture

3. Post Processing
   m_shaderResourceView(tempTexture SRV)를 첫 필터 입력으로 사용

4. Combine
   마지막 필터 결과를 m_renderTargetView(backBuffer RTV)에 출력

5. ImGui render

6. Present
```

이 구조에서는 `m_tempTexture`와 `m_shaderResourceView`가 back buffer 크기에 맞춰 항상 새로 유지되어야 한다.

---

## 왜 Post Processing OFF에서는 괜찮고 ON에서만 터졌는가

Post Processing OFF:

```text
scene -> backBuffer RTV
Present
```

Post Processing ON:

```text
scene -> backBuffer RTV
backBuffer -> tempTexture
tempTexture SRV -> filter chain
filter chain -> backBuffer RTV
Present
```

후처리를 켜면 back buffer 주변 리소스가 늘어난다.

| 상태 | back buffer 관련 참조 |
|---|---|
| Post Processing OFF | RTV 중심 |
| Post Processing ON | RTV + tempTexture + SRV + filter chain |

따라서 ON 상태에서 리사이즈할 때 `ResizeBuffers()` 실패 가능성이 훨씬 커졌다.

---

## 이번 버그에서 배운 점

| 배운 점 | 설명 |
|---|---|
| `ResizeBuffers()` 전에는 back buffer 관련 참조를 끊어야 한다 | RTV/SRV/depth/temp texture와 pipeline binding을 정리해야 한다 |
| 후처리 리소스는 화면 크기에 강하게 의존한다 | 리사이즈 후 filter chain 재생성이 필요하다 |
| back buffer를 직접 SRV로 만들 필요가 없는 구조라면 만들지 않는다 | 불필요한 참조가 resize 실패 원인이 될 수 있다 |
| D3D state는 렌더 패스 사이에서 유지된다 | 후처리 패스가 viewport를 바꾸면 메인 패스에서 다시 바인딩해야 한다 |
| HRESULT를 확인해야 원인을 좁힐 수 있다 | `887a0001` 로그가 원인 추적의 결정적 단서가 되었다 |

---

## 최종 상태

| 항목 | 결과 |
|---|---|
| Post Processing OFF 리사이즈 | 정상 |
| Post Processing ON 리사이즈 | 정상 |
| `ResizeBuffers() failed. 887a0001` | 해결 |
| 검정 화면/검정 잔상 | 해결 |
| 렌더링 영역 축소 | 해결 |
| 마우스 좌표 어긋남 | 렌더링 기준 정상화 후 함께 개선됨 |

