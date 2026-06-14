#pragma once
// 기본 메쉬 렌더링과 노멀 디버그 렌더링에 사용하는 constant buffer 데이터 정의.
// C++ 구조체와 HLSL cbuffer의 레이아웃이 맞도록 16바이트 정렬을 확인한다.

#include <directxtk/SimpleMath.h>

#include "ClassicLitTypes.h"
#include "Light.h"
#include "Material.h"

namespace Ryudar::ClassicLit
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

#pragma region Vertex Constant Data
struct VertexConstantData
{
	Matrix modelWorld;
	Matrix invTranspose;
	Matrix view;
	Matrix projection;
}; // 64byte * 4 = 256byte
static_assert((sizeof(VertexConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
#pragma endregion

#pragma region Pixel Constant Data
struct LightingConstantData
{
	Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS
	Vector3 eyeWorld{};       // 12
	float padding = 0.f;      // 4
};
static_assert((sizeof(LightingConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(LightingConstantData) == 160,
              "LightingConstantData must match the HLSL cbuffer layout");

// RimLight 관련 상수 버퍼 데이터 구조체. ShadingConstantData에서 관리한다.
struct RimLightConstantData
{
	Vector3 rimColor = Vector3(1.0f); // 12
	float power = 1.0f;               // 4
	float strength = 0.0f;            // 4
	uint32_t enabled = false;         // 4
	uint32_t useSmoothstep = false;   // 4
	float padding = 0.f;              // 4
};
static_assert((sizeof(RimLightConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(RimLightConstantData) == 32,
              "RimLightConstantData must match the HLSL cbuffer layout");

// Shading 옵션을 별도의 구조체로 관리한다. ShadingConstantData에서 관리한다.
struct ShadingOptionsConstantData
{
	uint32_t useTexture = false;                                        // 4
	uint32_t shadingModel = static_cast<uint32_t>(ShadingModel::Phong); // 4
	float padding[2] = {};                                              // 8
};
static_assert((sizeof(ShadingOptionsConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(ShadingOptionsConstantData) == 16,
              "ShadingOptionsConstantData must match the HLSL cbuffer layout");

// 환경 설정을 별도의 구조체로 관리한다. ShadingConstantData에서 관리한다.
struct EnvironmentConstantData
{
	uint32_t useIBL = false;                   // 4
	uint32_t useEnvironmentReflection = false; // 4
	float padding[2] = {};                     // 8
};
static_assert((sizeof(EnvironmentConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(EnvironmentConstantData) == 16,
              "EnvironmentConstantData must match the HLSL cbuffer layout");

// ClassicLit 렌더링에 필요한 모든 픽셀 셰이딩 상수 데이터를 하나의 구조체로 묶는다.
struct ShadingConstantData
{
	Material material;                         // 64
	RimLightConstantData rimLight;             // 32
	ShadingOptionsConstantData shadingOptions; // 16
	EnvironmentConstantData environment;       // 16
};
static_assert((sizeof(ShadingConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(ShadingConstantData) == 128,
              "ShadingConstantData must match the HLSL cbuffer layout");
#pragma endregion

} // namespace Ryudar::ClassicLit

namespace Ryudar
{

struct NormalVertexConstantData
{
	float scale = 0.1f;
	float dummy[3];
};

static_assert((sizeof(NormalVertexConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(NormalVertexConstantData) == 16,
              "NormalVertexConstantData must match the HLSL cbuffer layout");

} // namespace Ryudar
