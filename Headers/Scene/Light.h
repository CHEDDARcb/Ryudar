#pragma once
// Directional Light、Point Light、Spot Lightで共通使用するLightデータ構造体。
// HLSL側のLight構造体およびConstant Bufferと同じフィールド順を維持する。

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

struct Light
{
	Vector3 strength = Vector3(1.0f);
	float fallOffStart = 0.0f;                     // Point/Spot Lightで使用
	Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // Directional/Spot Lightで使用
	float fallOffEnd = 10.0f;                      // Point/Spot Lightで使用
	Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // Point/Spot Lightで使用
	float spotPower = 100.0f;                      // Spot Lightで使用
}; // 48byte
} // namespace Ryudar
