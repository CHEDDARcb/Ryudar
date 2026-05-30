#pragma once
// 기본 메쉬 렌더링과 노멀 디버그 렌더링에 사용하는 constant buffer 데이터 정의.
// C++ 구조체와 HLSL cbuffer의 레이아웃이 맞도록 16바이트 정렬을 확인한다.

#include <directxtk/SimpleMath.h>

#include "Light.h"
#include "Material.h"

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

struct BasicVertexConstantData
{
	Matrix modelWorld;
	Matrix invTranspose;
	Matrix view;
	Matrix projection;
}; // 64byte * 4 = 256byte

static_assert((sizeof(BasicVertexConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

struct BasicPixelConstantData
{
	Vector3 eyeWorld;                 // 12
	bool useTexture;                  // 4 `
	Material material;                // 48 `
	Light lights[MAX_LIGHTS];         // 48 * MAX_LIGHTS `
	Vector3 rimColor = Vector3(1.0f); // 12
	float rimPower;                   // 4 `
	float rimStrength = 0.0f;         // 4
	bool useSmoothstep = false;       // 4
	uint32_t useBlinnPhong = false;   // 4
	uint32_t usePhong = true;         // 4 `
	bool useRimLight = false;         // 4
	float padding0[3];                // 12 `
	bool useEnvironmentReflection;    // 4
	float padding1[3];                // 12 `
	bool useIBL = false;              // 4
	float padding2[3];                // 12 `
};

static_assert((sizeof(BasicPixelConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

struct NormalVertexConstantData
{
	float scale = 0.1f;
	float dummy[3];
};

static_assert((sizeof(NormalVertexConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

} // namespace Ryudar
