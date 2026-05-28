#pragma once

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
}; // 64byte

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
	float dummy1[3];                  // 12 `
	bool useEvMapping;                // 4
	float dummy2[3];                  // 12 `
	bool useIBL = false;              // 4
	float dummy3[3];                  // 12 `
};

static_assert((sizeof(BasicPixelConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

struct NormalVertexConstantData
{
	float scale = 0.1f;
	float dummy[3];
};

} // namespace Ryudar