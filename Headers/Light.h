#pragma once
// Direct Light, Point Light, Spot Light 계산에 공통으로 사용하는 조명 데이터 구조체.
// HLSL Light 구조체와 constant buffer 레이아웃이 맞도록 구성한다.

#include <cstddef>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

enum class LightType
{
	Directional = 0,
	Point = 1,
	Spot = 2,
	Count
};

inline constexpr std::size_t MaxLights = static_cast<std::size_t>(LightType::Count);

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