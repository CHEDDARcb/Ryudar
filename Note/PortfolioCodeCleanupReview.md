# Portfolio Code Cleanup Review

작성일: 2026-05-29  
목적: 포트폴리오 제출 전, 큰 리팩터링 없이 코드 인상을 빠르게 개선할 수 있는 지점 정리

---

## 한 줄 요약

현재 프로젝트는 `AppBase -> Ryudar` 구조로 Win32/D3D11 앱 기반과 데모 씬 로직이 분리되어 있고, `BasicMeshGroup`, `CubeMapping`, `ImageFilter`, `Camera` 등 주요 기능 단위도 어느 정도 나뉘어 있습니다.  
다만 포트폴리오 관점에서는 **상태 소유권**, **public 멤버 노출**, **네이밍/오타**, **후처리 필터 구성**, **리사이즈 대응** 쪽을 조금만 정리해도 훨씬 안정적인 코드처럼 보입니다.

---

## 우선순위별 정리 목록

| 우선순위 | 항목 | 위치 | 이유 | 빠른 개선 방향 |
|---|---|---|---|---|
| P0 | Blur Y 패스가 `BlurX` 셰이더를 사용함 | `Sources/Ryudar.cpp:298` | 블룸 설명은 X/Y 블러인데 실제 코드는 Y 패스도 X 블러입니다. 결과 품질/의도 불일치로 보일 수 있습니다. | 두 번째 블러 필터 생성 시 `L"BlurY"` 사용 |
| P0 | 윈도우 리사이즈 후 필터/카메라 aspect 갱신 누락 | `Sources/AppBase.cpp:155`, `Sources/Ryudar.cpp:254` | 렌더타겟 크기는 바뀌지만 `ImageFilter` 해상도와 카메라 aspect가 이전 값에 머물 수 있습니다. | `OnResize()` 가상 함수 추가 또는 `WM_SIZE`에서 카메라 aspect 갱신, 필터 재빌드 |
| P1 | `static int changedMesh` 전역 상태 | `Sources/Ryudar.cpp:70` | 씬 상태가 클래스 밖에 있어 소유권이 흐려집니다. 포폴 코드에서 전역 상태는 눈에 잘 띕니다. | `Ryudar` 멤버 `bool m_meshSelectionChanged`로 이동 |
| P1 | `AppBase`, `BasicMeshGroup`의 public 멤버 과다 노출 | `Headers/AppBase.h:52`, `Headers/BasicMeshGroup.h:25` | 외부에서 내부 렌더링 리소스를 마음대로 바꿀 수 있어 캡슐화가 약해 보입니다. | 단기적으로는 주석 정리, 중기적으로 getter/setter 또는 설정 함수로 축소 |
| P1 | GUI 라벨/변수명 오타 | `Sources/Ryudar.cpp:387`, `Sources/Ryudar.cpp:388`, `Headers/imageFilter.h:185` | 작은 오타가 포폴 완성도를 크게 낮춰 보이게 합니다. | `Translastion -> Translation`, `Rotaion -> Rotation`, `m_rasterizerSate -> m_rasterizerState` |
| P1 | 헤더 파일 대소문자 불일치 | `Headers/Ryudar.h:11`, `Headers/imageFilter.h` | Windows에서는 빌드될 수 있지만, Git/CI/다른 환경에서 깨질 수 있습니다. | 파일명을 `ImageFilter.h`로 통일하거나 include를 실제 파일명과 일치 |
| P2 | `Ryudar` 클래스가 씬, GUI, 후처리, 모델 선택을 모두 담당 | `Headers/Ryudar.h`, `Sources/Ryudar.cpp` | 기능은 잘 보이지만 클래스 책임이 많아 보입니다. | 시간이 있으면 `SceneSettings`, `PostProcessSettings`, `Draw*GUI()` 묶음 유지 |
| P2 | `D3D11Utils` 실패 처리가 출력만 하고 계속 진행 | `Sources/D3D11Utils.cpp`, `Headers/D3D11Utils.h` | 실패 후 null 리소스 접근 가능성이 있어 안정성 질문을 받을 수 있습니다. | 생성 함수가 `bool`/`HRESULT`를 반환하게 바꾸기 |
| P2 | `using namespace std;`가 cpp 전역에 여러 번 등장 | `Sources/Ryudar.cpp:12`, `Sources/AppBase.cpp:18` | 치명적이지는 않지만 포폴 코드에서는 명시성이 더 깔끔합니다. | 자주 쓰는 타입만 `using std::...` 또는 직접 `std::` 사용 |
| P3 | 오래된 주석/주석 처리된 코드가 많음 | `Sources/Ryudar.cpp`, `Sources/ModelLoader.cpp` | 학습 흔적은 좋지만 제출용 코드에서는 산만해 보일 수 있습니다. | 설명 가치가 있는 주석만 남기고 실험 코드 제거 |

---

## 바로 하면 좋은 Quick Wins

| 작업 | 예상 시간 | 체감 효과 |
|---|---:|---|
| GUI 라벨 오타 수정 | 5분 | 데모 실행 화면 완성도 상승 |
| `BlurY` 셰이더 사용 누락 수정 | 5분 | 블룸 구현 신뢰도 상승 |
| `changedMesh`를 `Ryudar` 멤버로 이동 | 10분 | 전역 상태 제거 |
| `ImageFilter.h` 파일명/include 대소문자 통일 | 5분 | 프로젝트 정돈감 상승 |
| 깨진 `MaterialOwnershipNotes.md`를 삭제하거나 UTF-8로 재작성 | 10분 | 저장소 첫인상 개선 |
| 디버그용 `std::cout` 출력 정리 | 10분 | 실행 로그 깔끔함 |

---

## 구조 리뷰

### 1. 상속 구조

현재 구조:

```text
AppBase
  - Win32 window
  - D3D11 device/context/swapchain
  - ImGui frame
  - input message
  - camera

Ryudar : AppBase
  - scene object selection
  - material/light GUI
  - cube mapping
  - bloom filter chain
  - render order
```

이 구조 자체는 포트폴리오용 데모 앱으로 자연스럽습니다.  
`AppBase`가 플랫폼/렌더링 초기화를 맡고, `Ryudar`가 실제 데모 씬을 구성하는 구조라 설명하기 좋습니다.

다만 `AppBase`의 public 멤버가 너무 많습니다. `m_device`, `m_context`, `m_swapChain`, `m_renderTargetView`까지 모두 public이라 파생 클래스뿐 아니라 외부에서도 내부 상태를 직접 만질 수 있습니다. 포폴에서는 아래처럼 설명 가능한 형태가 더 좋습니다.

```text
AppBase
  protected:
    device/context/render target
  public:
    Initialize(), Run(), GetAspectRatio()
```

시간이 부족하면 실제 리팩터링까지 하지 않아도, 최소한 public/protected 범위만 조정하거나 "파생 클래스에서 렌더링 리소스 접근을 위해 protected로 관리"하는 방향으로 정리하면 좋습니다.

---

### 2. 상태 소유권

#### `changedMesh`

`Sources/Ryudar.cpp:70`의 `static int changedMesh`는 `Ryudar` 클래스 밖에 있습니다.

문제점:

| 현재 | 아쉬운 점 |
|---|---|
| 파일 전역 static 변수 | 씬 상태인데 클래스가 소유하지 않음 |
| `int` 플래그 | 의미가 true/false인데 타입이 명확하지 않음 |
| `DrawMeshSelectorGUI()`에서 변경, `Update()`에서 소비 | 흐름은 좋지만 소유 위치가 애매함 |

추천:

```cpp
bool m_meshSelectionChanged = false;
```

이렇게 `Ryudar` 멤버로 두면 "GUI에서 상태를 변경하고, Update에서 변경된 상태를 반영한다"는 흐름이 깔끔해집니다.

#### Material 소유권

현재 `BasicMeshGroup`이 `m_basicPixelConstantData.material`을 가지고 있고, GUI가 선택된 `BasicMeshGroup`의 material을 직접 수정합니다. 이 방향은 괜찮습니다.  
오히려 material 값을 `Ryudar` 전역 멤버로 다시 빼는 것보다, **각 MeshGroup이 자기 material을 가진다**는 점이 설명하기 좋습니다.

개선 방향:

| 현재 | 개선 |
|---|---|
| `BasicPixelConstantData` 안의 GPU 전송용 material을 GUI가 직접 수정 | `EditableMaterial` 같은 CPU 편집용 구조체를 두고 GPU용 `Material`로 변환 |
| diffuse/specular를 평균 slider 하나로 조절 | 색상은 ColorEdit, 강도는 Slider로 분리 |

이건 시간이 있을 때 하면 좋고, 지금은 "material은 MeshGroup 단위로 소유"만 유지해도 충분합니다.

---

### 3. `BasicMeshGroup` 책임

`BasicMeshGroup`은 현재 아래 역할을 같이 합니다.

| 역할 | 현재 위치 |
|---|---|
| 모델/메시 로드 | `Initialize()` |
| vertex/index buffer 생성 | `Initialize()` |
| 기본 셰이더 생성 | `Initialize()` |
| material/light constant buffer 보유 | public member |
| normal debug line 생성/렌더링 | `Initialize()`, `Render()` |
| 실제 draw call | `Render()` |

포트폴리오 설명에서는 "하나의 렌더링 가능한 MeshGroup 단위"라고 말하기 좋지만, public 데이터가 많아서 내부 구현이 노출되어 보입니다.

단기 개선:

| 항목 | 방향 |
|---|---|
| `m_basicPixelConstantData` public | 당장 유지하되 주석으로 "GUI 편집을 위해 임시 공개" 같은 의도 명시 |
| texture/cubemap SRV public | `SetIBLResources(diffuse, specular)` 함수로 감싸기 |
| normal debug 관련 public | `SetDrawNormals(bool)`, `SetNormalScale(float)` 함수로 감싸기 |

중기 개선:

```text
BasicMeshGroup
  SetMaterial()
  SetLights()
  SetIBLResources()
  SetTransform()
  UpdateConstantBuffers()
  Render()
```

이렇게 외부 API가 동작 중심으로 보이면 훨씬 엔진 코드처럼 보입니다.

---

## 렌더링/후처리 이슈

### 1. Blur Y 패스 오타 가능성

`BuildFilters()`에서 X축 블러와 Y축 블러를 만든다고 주석이 되어 있습니다.

```cpp
// x축 블러처리
L"BlurX"

// y축 블러처리
L"BlurX"
```

두 번째는 `L"BlurY"`가 맞아 보입니다. `Shaders/BlurYPixelShader.hlsl`도 존재하므로 단순 실수일 가능성이 큽니다.

추천:

```cpp
m_filters.push_back(make_shared<ImageFilter>(
    m_device, m_context, L"Sampling", L"BlurY",
    m_screenWidth / down, m_screenHeight / down));
```

### 2. 리사이즈 대응

`WM_SIZE`에서 swap chain, render target, depth buffer, viewport는 갱신합니다.  
하지만 아래 리소스들은 초기 해상도를 기준으로 만들어진 뒤 유지됩니다.

| 리소스 | 생성 위치 | 문제 |
|---|---|---|
| Bloom `ImageFilter` 체인 | `Ryudar::BuildFilters()` | 리사이즈 후 필터 텍스처 크기가 이전 크기 |
| Camera aspect | `AppBase::AppBase()` | 리사이즈 후 projection aspect가 이전 값 |

추천 구조:

```text
AppBase::MsgProc(WM_SIZE)
  Resize swapchain/render target/depth
  SetViewport()
  OnResize()

Ryudar::OnResize()
  m_camera.SetAspectRatio(GetAspectRatio())
  BuildFilters()
```

이렇게 만들면 "리소스 라이프사이클을 윈도우 이벤트에 맞춰 관리했다"고 설명하기 좋습니다.

### 3. 후처리 파라미터 dirty flag

`m_dirtyflag`는 동작은 하지만 이름과 타입이 아쉽습니다.

| 현재 | 추천 |
|---|---|
| `int m_dirtyflag = 1` | `bool m_postProcessDirty = true` |
| `m_dirtyflag += ImGui::SliderFloat(...)` | `m_postProcessDirty |= ImGui::SliderFloat(...)` |

의미가 명확해지고, C++ 코드 리뷰에서 지적받을 확률이 줄어듭니다.

---

## 네이밍/오타 체크

| 현재 | 추천 | 위치 |
|---|---|---|
| `m_modelTranslastion` | `Model Translation` 또는 `m_modelTranslation` | `Sources/Ryudar.cpp:387` |
| `m_modelRotaion(Rad)` | `Model Rotation (Rad)` | `Sources/Ryudar.cpp:388` |
| `m_rasterizerSate` | `m_rasterizerState` | `Headers/imageFilter.h:185` |
| `useEvMapping` | `useEnvMapping` | `Headers/BasicConstantData.h:39`, `Shaders/BasicPixelShader.hlsl:22` |
| `CrteateBuffer()` | `CreateBuffer()` | `Headers/D3D11Utils.h:73` |
| `dirtyflag` | `dirtyFlag` 또는 `postProcessDirty` | `Headers/Ryudar.h:66` |
| `m_down` | `m_downsampleFactor` | `Headers/Ryudar.h:67` |
| `m_repeat` | `m_blurRepeatCount` | `Headers/Ryudar.h:68` |

GUI에 보이는 오타는 특히 먼저 고치는 것을 추천합니다. 실행 화면 캡처나 시연에서 바로 드러납니다.

---

## 파일/빌드 정리

### 1. `ImageFilter.h` 대소문자

`Ryudar.h`에서는 `#include "ImageFilter.h"`를 사용하지만 실제 파일은 `Headers/imageFilter.h`입니다.  
Windows 파일 시스템에서는 통과할 수 있지만, 대소문자를 구분하는 환경에서는 문제가 됩니다.

추천:

```text
Headers/imageFilter.h -> Headers/ImageFilter.h
Ryudar.vcxproj의 ClInclude 경로도 같이 수정
```

### 2. 깨진 문서 인코딩

루트의 `MaterialOwnershipNotes.md`는 현재 한글이 깨져 보입니다. 포트폴리오 제출용 저장소라면 아래 중 하나를 추천합니다.

| 선택지 | 추천도 |
|---|---|
| UTF-8로 재작성해서 `Note/MaterialOwnershipNotes.md`로 이동 | 높음 |
| 이 문서에 내용 통합 후 기존 파일 삭제 | 높음 |
| 그대로 유지 | 낮음 |

---

## 제출 전 최소 정리 체크리스트

- [ ] GUI 라벨 오타 수정
- [ ] `BlurY` 필터 사용 누락 수정
- [ ] `changedMesh`를 `Ryudar` 멤버 bool로 이동
- [ ] `ImageFilter.h` 파일명/include 대소문자 통일
- [ ] `m_dirtyflag`를 `m_postProcessDirty`로 변경
- [ ] 리사이즈 시 `m_camera.SetAspectRatio()` 호출
- [ ] 시간이 되면 리사이즈 시 `BuildFilters()` 재호출
- [ ] 디버그용 `std::cout` 정리
- [ ] 깨진 `MaterialOwnershipNotes.md` 정리
- [ ] README에 실행 방법, 조작법, 구현 기능, 스크린샷 추가

---

## 포트폴리오 설명 포인트

면접이나 코드 설명에서 아래처럼 말하면 좋습니다.

> `AppBase`는 Win32 메시지 루프, D3D11 디바이스/스왑체인, ImGui 프레임 같은 애플리케이션 공통 기반을 담당하고, `Ryudar`는 실제 렌더링 데모 씬을 구성합니다. 메시, 큐브맵, 후처리 필터, 카메라는 각각 별도 클래스로 분리했습니다.

> 제출 전 정리 과정에서는 전역 상태를 클래스 멤버로 옮기고, 리사이즈 시 GPU 리소스 갱신 흐름을 보강하고, GUI와 셰이더 이름의 오타를 정리해서 유지보수성을 높였습니다.

> Material 값은 전역 상태가 아니라 렌더링 대상인 `BasicMeshGroup`이 소유하게 두는 것이 자연스럽다고 판단했습니다. 이후 확장한다면 CPU 편집용 material과 GPU constant buffer용 material을 분리할 계획입니다.

---

## 시간이 조금 더 있을 때 할 만한 개선

| 개선 | 효과 |
|---|---|
| `AppBase::OnResize()` 가상 함수 추가 | 리소스 라이프사이클 구조가 명확해짐 |
| `BasicMeshGroup` public 데이터 축소 | 캡슐화 개선 |
| `ImageFilter`를 `.h/.cpp`로 분리 | 헤더 의존성과 빌드 단위 정리 |
| `D3D11Utils` 함수들이 `HRESULT`/`bool` 반환 | 실패 처리 안정성 개선 |
| `enum class MeshSelection`, `enum class LightType` 추가 | magic number 제거 |
| `PostProcessSettings` 구조체 추가 | 블룸 관련 변수 묶음 정리 |
| README 작성 | 포폴 완성도 상승 |

