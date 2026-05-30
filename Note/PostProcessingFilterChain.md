# Post Processing Filter Chain

작성일: 2026-05-30  
범위: `BuildFilters()`, `ImageFilter`, `m_tempTexture`, SRV/RTV 연결 구조 이해 정리

---

## 핵심 요약

`BuildFilters()`는 후처리를 실제로 실행하는 함수가 아니다.  
후처리에 필요한 필터 객체와 GPU 리소스, 입력/출력 연결 관계를 미리 구성하는 함수다.

실제 후처리는 매 프레임 `Ryudar::Render()`에서 실행된다.

```text
BuildFilters()
  필터 체인 준비

Render()
  현재 프레임 렌더링 결과를 resolve
  m_filters를 순서대로 Render()
```

---

## 전체 흐름

```text
1. Scene Render
   cube map, sphere/character 등을 back buffer에 렌더링

2. Resolve
   MSAA back buffer 결과를 m_tempTexture로 복사

3. Copy Filter
   m_tempTexture SRV를 읽어서 후처리 체인 내부 원본으로 복사

4. Downsample
   해상도를 낮춘 texture들 생성

5. Blur
   낮은 해상도 이미지에 blur 적용

6. Combine
   원본 이미지 + blur 결과를 합쳐 back buffer에 출력

7. ImGui Render

8. Present
```

---

## `m_tempTexture`와 `m_shaderResourceView`

### 역할

| 리소스 | 역할 |
|---|---|
| `backBuffer` | 실제 화면에 표시될 swap chain buffer |
| `m_tempTexture` | 후처리 셰이더가 읽을 수 있도록 back buffer 내용을 복사해 둔 texture |
| `m_shaderResourceView` | `m_tempTexture`를 pixel shader에서 읽기 위한 SRV |

### 왜 `m_tempTexture`가 필요한가

현재 scene은 swap chain back buffer에 먼저 렌더링된다.

하지만 후처리 shader가 back buffer를 직접 읽는 구조는 안전하지 않다.

이유:

| 이유 | 설명 |
|---|---|
| MSAA resolve 필요 | MSAA back buffer는 일반 후처리 shader에서 바로 읽기 어렵다 |
| 읽기/쓰기 충돌 방지 | 같은 리소스를 render target으로 쓰면서 동시에 shader resource로 읽으면 안 된다 |

그래서 `Render()`에서 후처리 직전에 아래 흐름을 수행한다.

```cpp
ComPtr<ID3D11Texture2D> backBuffer;
m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));

m_context->ResolveSubresource(
    m_tempTexture.Get(), 0,
    backBuffer.Get(), 0,
    DXGI_FORMAT_R8G8B8A8_UNORM);
```

정리:

```text
backBuffer
  현재 프레임의 scene 렌더링 결과

m_tempTexture
  후처리 입력으로 쓰기 위한 원본 복사본

m_shaderResourceView
  m_tempTexture를 shader에서 읽기 위한 view
```

---

## `BuildFilters()`는 무엇을 하나

`BuildFilters()`는 필터를 실행하지 않는다.  
필터 체인을 미리 만들어서 `m_filters`에 순서대로 저장한다.

```text
BuildFilters()
  - ImageFilter 객체 생성
  - 각 필터 내부 texture/RTV/SRV 생성
  - 각 필터의 입력 SRV 지정
  - 마지막 combine filter의 출력 RTV 지정
```

실제 실행은 여기서 일어난다.

```cpp
if (m_usePostProcessing)
{
    for (auto &f : m_filters)
    {
        f->Render(m_context);
    }
}
```

즉:

```text
BuildFilters()
  설계도와 작업대 준비

ImageFilter::Render()
  실제 GPU 작업 실행
```

---

## `ImageFilter` 하나의 의미

`ImageFilter` 하나는 작은 렌더링 패스다.

```text
입력 SRV
  -> vertex/pixel shader
  -> render target에 출력
  -> 출력 texture를 SRV로 다음 필터에 제공
```

각 `ImageFilter`는 기본적으로 자기 내부 texture를 만들고, 그 texture에 대한 RTV/SRV를 가진다.

```text
ImageFilter
  input SRV 목록
  internal texture
  internal RTV
  internal SRV
  viewport
  shader
```

---

## 필터 체인 순서

`m_down = 16`이라고 하면 downsample 필터는 `/2`, `/4`, `/8`, `/16` 네 개가 만들어진다.

`m_down / 2`개가 아니라 `log2(m_down)`개라고 보는 것이 정확하다.

예시:

```text
m_filters[0]
  copyFilter

m_filters[1]
  downsample /2

m_filters[2]
  downsample /4

m_filters[3]
  downsample /8

m_filters[4]
  downsample /16

이후
  blur X
  blur Y
  upsample
  blur X
  blur Y
  upsample
  ...

마지막
  combineFilter
```

---

## Copy Filter

```cpp
auto copyFilter = make_shared<ImageFilter>(
    m_device, m_context,
    L"Sampling", L"Sampling",
    m_screenWidth, m_screenHeight);

copyFilter->SetShaderResources({this->m_shaderResourceView});
m_filters.push_back(copyFilter);
```

역할:

```text
m_shaderResourceView
  m_tempTexture를 읽는 SRV

copyFilter
  현재 프레임 원본 이미지를 후처리 체인 내부 texture로 복사
```

copyFilter의 출력은 이후 `copyFilter->m_shaderResourceView`로 읽을 수 있다.

---

## Downsample Filters

다운샘플 필터는 이전 필터의 SRV를 입력으로 받는다.

```cpp
downFilter->SetShaderResources({m_filters.back()->m_shaderResourceView});
```

개념:

```text
copyFilter SRV
  -> down /2
      -> down /4
          -> down /8
              -> down /16
```

각 down filter는 자기 내부 texture에 작은 해상도의 결과를 출력한다.

---

## Blur / Up Filters

Blur filter도 같은 구조다.

```text
입력: 이전 필터의 SRV
출력: 자기 내부 texture
```

흐름:

```text
lowest downsample result
  -> BlurX
      -> BlurY
          -> Upsample
              -> BlurX
                  -> BlurY
                      -> Upsample
```

각 단계는 `m_filters.back()->m_shaderResourceView`를 입력으로 받아서 체인처럼 이어진다.

---

## Combine Filter

Combine filter는 입력 SRV를 두 개 사용한다.

```cpp
combineFilter->SetShaderResources(
    {copyFilter->m_shaderResourceView,
     m_filters.back()->m_shaderResourceView});
```

| 입력 | 의미 |
|---|---|
| `copyFilter->m_shaderResourceView` | 원본 scene 복사본 |
| `m_filters.back()->m_shaderResourceView` | 마지막 blur/up 필터 결과 |

개념:

```text
original scene
  +
blurred bright image
  =
bloom result
```

---

## 왜 Combine Filter만 `SetRenderTargets()`를 따로 호출하나

일반 필터들은 생성될 때 자기 내부 render target을 출력 대상으로 설정한다.

```cpp
this->SetRenderTargets({m_renderTargetView});
```

여기서 `m_renderTargetView`는 `ImageFilter` 자기 자신의 내부 texture에 대한 RTV다.

일반 필터:

```text
입력 SRV
  -> shader
  -> 자기 내부 RTV에 출력
```

하지만 combine filter는 최종 결과를 화면에 보여줘야 한다.

그래서 combine filter만 출력 render target을 화면용 RTV로 바꾼다.

```cpp
combineFilter->SetRenderTargets({this->m_renderTargetView});
```

여기서 `this->m_renderTargetView`는 `Ryudar/AppBase`의 swap chain back buffer RTV다.

정리:

| 필터 | 출력 대상 |
|---|---|
| copy/down/blur/up filter | 각 필터 내부 texture |
| combine filter | 화면 back buffer RTV |

---

## `SetShaderResources()`가 여러 SRV를 받는 방식

함수 인수는 하나다.

```cpp
void SetShaderResources(
    const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources)
```

하지만 그 하나의 인수가 `vector`이기 때문에 여러 SRV를 담을 수 있다.

```cpp
combineFilter->SetShaderResources(
    {copyFilter->m_shaderResourceView,
     m_filters.back()->m_shaderResourceView});
```

위 코드는 개념적으로 아래와 같다.

```cpp
std::vector<ComPtr<ID3D11ShaderResourceView>> resources;
resources.push_back(copyFilter->m_shaderResourceView);
resources.push_back(m_filters.back()->m_shaderResourceView);

combineFilter->SetShaderResources(resources);
```

렌더링할 때는 vector에 들어 있는 SRV raw pointer들을 pixel shader에 바인딩한다.

```cpp
context->PSSetShaderResources(
    0,
    UINT(m_shaderResources.size()),
    m_shaderResources.data());
```

combine filter 기준:

```text
t0 = original scene copy
t1 = final blur result
```

---

## 한 장으로 보는 연결 구조

```text
backBuffer
  scene rendering result
      |
      | ResolveSubresource
      v
m_tempTexture
      |
      | m_shaderResourceView
      v
copyFilter
      |
      v
downsample filters
      |
      v
blur / up filters
      |
      v
last filter SRV

combineFilter inputs:
  1. copyFilter SRV
  2. last filter SRV

combineFilter output:
  backBuffer RTV
```

---

## 최종 이해 문장

> `BuildFilters()`는 후처리 작업을 실행하는 함수가 아니라, 매 프레임 실행될 후처리 렌더 패스들의 연결 구조를 미리 구성하는 함수다. 실제 후처리는 `Ryudar::Render()`에서 현재 프레임의 back buffer를 `m_tempTexture`로 resolve한 뒤, `m_filters`에 들어 있는 `ImageFilter`들을 순서대로 렌더링하면서 수행된다. 마지막 `combineFilter`만 화면 back buffer에 출력하고, 나머지 필터들은 자기 내부 texture에 중간 결과를 저장한다.

