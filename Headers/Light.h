#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#define MAX_LIGHTS 3

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// 조명
struct Light
{
	Vector3 strength = Vector3(1.0f);
	float fallOffStart = 0.0f;                     // point/spot light only
	Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // directional / spot light only
	float fallOffEnd = 10.0f;                      // point/spot light only
	Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // point/spot light only
	float spotPower = 100.0f;                      // spot light only
}; // 48byte
} // namespace Ryudar