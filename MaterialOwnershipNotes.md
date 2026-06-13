# 렌더러 Material 소유권 정리

## 현재 코드에서 발견한 문제

Material 속성은 소유자가 명확해야 한다.

이전 구조에서는 `diffuse`, `specular`는 `Ryudar` 멤버 변수로 관리했고,
`shininess`는 `meshGroup.m_pixelConstantData.material`을 직접 수정했다.

즉 상태 소유권이 다음처럼 섞여 있었다.

| 속성 | 소유 위치 | 결과 |
|---|---|---|
| Diffuse | `Ryudar` | 구체와 캐릭터가 같은 값 공유 |
| Specular | `Ryudar` | 구체와 캐릭터가 같은 값 공유 |
| Shininess | `ClassicLit::MeshGroup`의 material | 구체와 캐릭터가 서로 다른 값 가능 |

`diffuse`, `specular`, `shininess`는 모두 물체 표면의 성질이다. 그런데 어떤 값은 전역처럼 공유되고, 어떤 값은 오브젝트별로 따로 관리되면 코드 의도가 흐려진다.

## 일반적인 렌더러 설계

대부분의 렌더러에서 Material 속성은 렌더러 전역 상태가 아니라 오브젝트, mesh, submesh 또는 material 인스턴스가 소유한다.

예를 들면 다음 구조가 자연스럽다.

```text
Sphere Material
  diffuse
  specular
  shininess

Character Material
  diffuse
  specular
  shininess
```

이렇게 하면 구체와 캐릭터가 서로 다른 표면 속성을 가질 수 있다.

## Single Source Of Truth

설계에서 중요한 기준은 single source of truth, 즉 실제 상태를 한 곳에 두는 것이다.

이 프로젝트에서는 material 값의 자연스러운 소유 위치가 다음이다.

```cpp
meshGroup.m_pixelConstantData.material
```

GUI는 이 실제 material 값을 수정하고, constant buffer 업데이트 과정에서 이 값이 GPU로 전달된다.

이 구조의 장점은 다음과 같다.

- UI 상태와 렌더링 상태가 불필요하게 분리되지 않는다.
- 구체와 캐릭터가 각각 자기 material 값을 가진다.
- `Update()`에서 별도 전역 값을 material로 덮어쓰는 흐름이 줄어든다.
- 코드를 읽을 때 “어떤 값이 진짜 렌더링에 쓰이는가?”가 명확하다.

## 현재 적용한 개선

현재는 GUI가 선택된 `ClassicLit::MeshGroup`의 material을 직접 수정한다.

```cpp
auto &material = meshGroup.m_pixelConstantData.material;

float diffuse = (material.diffuse.x + material.diffuse.y + material.diffuse.z) / 3.0f;
if (ImGui::SliderFloat("Material Diffuse", &diffuse, 0.0f, 3.0f)) {
    material.diffuse = Vector3(diffuse);
}

float specular = (material.specular.x + material.specular.y + material.specular.z) / 3.0f;
if (ImGui::SliderFloat("Material Specular", &specular, 0.0f, 3.0f)) {
    material.specular = Vector3(specular);
}

ImGui::SliderFloat("Material Shininess", &material.shininess, 0.01f, 20.0f);
```

`diffuse`와 `specular`는 내부적으로 `Vector3`이지만, GUI에서는 하나의 슬라이더로 조절한다. 슬라이더 값이 바뀌면 RGB 세 채널에 같은 값을 넣는다.

이 방식은 다음 장점이 있다.

- 기존처럼 밝기/세기를 한 번에 조절할 수 있다.
- material 소유권은 `ClassicLit::MeshGroup`에 유지된다.
- 구체와 캐릭터가 서로 다른 material 값을 가질 수 있다.
- HLSL 구조체는 수정하지 않아도 된다.

## 색상과 강도를 분리하는 방향

실제 모델링 툴이나 엔진의 material editor는 보통 색상과 강도를 분리한다.

예를 들면 다음과 같은 형태다.

```text
Diffuse Color + Diffuse Strength
Specular Color + Specular Strength
Shininess 또는 Roughness
```

이 구조를 적용하면 색은 color picker로 고르고, 전체 강도는 slider로 조절할 수 있다.

다만 이때 주의할 점이 있다. 편집용 material을 `Ryudar`에 하나만 두면 다시 모든 오브젝트가 같은 값을 공유하게 된다.

따라서 편집용 material도 오브젝트별 또는 material 인스턴스별로 소유해야 한다.

```text
Sphere
  EditableMaterial
  GPU Material

Character
  EditableMaterial
  GPU Material
```

## HLSL과의 관계

현재 C++의 `Material` 구조체는 HLSL의 `Material` 구조체와 constant buffer layout을 맞추고 있다.

C++:

```cpp
struct Material
{
    Vector3 ambient;
    float shininess;
    Vector3 diffuse;
    float dummy1;
    Vector3 specular;
    float dummy2;
    Vector3 fresnelR0;
    float dummy3;
};
```

HLSL:

```hlsl
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1;
    float3 specular;
    float dummy2;
    float3 fresnelR0;
    float dummy3;
};
```

따라서 GPU로 보내는 `Material` 구조체 자체를 바꾸면 HLSL 구조체도 같이 바꿔야 한다.

HLSL을 그대로 유지하고 싶다면 C++에서 두 계층으로 나누는 방식이 좋다.

```text
C++ 편집용 EditableMaterial
  diffuseColor
  diffuseStrength
  specularColor
  specularStrength

GPU로 보내는 Material
  diffuse = diffuseColor * diffuseStrength
  specular = specularColor * specularStrength
```

이렇게 하면 shader의 Phong/Blinn-Phong 계산 로직은 그대로 두면서, C++의 UI/editor 모델만 더 편하게 확장할 수 있다.

## 면접에서 설명하는 방식

면접에서는 다음처럼 설명할 수 있다.

> Diffuse, specular, shininess는 렌더러 전역 상태라기보다 물체 표면의 성질이라고 봅니다. 그래서 material 또는 mesh가 소유하는 것이 자연스럽습니다.

현재 코드의 문제는 다음처럼 말할 수 있다.

> 이전 구조에서는 diffuse/specular는 `Ryudar` 멤버 변수로 공유되고, shininess는 `meshGroup` 내부 material을 직접 수정했습니다. 그래서 material 속성마다 소유권이 달라져 의도가 애매했습니다.

개선 방향은 다음처럼 말할 수 있다.

> Material 관련 값은 하나의 material 데이터가 소유하게 하고, GUI는 그 실제 상태를 수정하도록 바꾸는 것이 더 일관된 설계라고 생각합니다. 이렇게 하면 UI 상태와 렌더링 상태가 분리되면서 생기는 동기화 문제를 줄이고, single source of truth를 유지할 수 있습니다.

짧게 정리하면 다음 문장이 핵심이다.

> Material 속성은 Material이 소유하게 하고, GUI는 그 실제 상태를 수정하게 만든다.

## 최종 요약

현재 개선된 구조:

```text
Diffuse / Specular / Shininess
  모두 meshGroup의 material 값
  -> 구체와 캐릭터가 서로 다른 값 가능
```

GUI 사용성:

```text
Diffuse / Specular
  내부는 RGB Vector3
  GUI에서는 단일 슬라이더로 한 번에 조절
```

장기 확장 방향:

```text
EditableMaterial을 오브젝트별로 추가
색상과 강도를 분리
GPU Material로 조합해서 HLSL에 전달
```
