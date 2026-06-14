#pragma once
// 방향광, 점광원, 스포트라이트 계산에 공통으로 사용하는 조명 데이터 구조체.
// HLSL 조명 구조체 및 상수 버퍼 레이아웃과 동일한 순서를 유지한다.

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
	float fallOffStart = 0.0f;                     // 점광원과 스포트라이트에서 사용
	Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 방향광과 스포트라이트에서 사용
	float fallOffEnd = 10.0f;                      // 점광원과 스포트라이트에서 사용
	Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 점광원과 스포트라이트에서 사용
	float spotPower = 100.0f;                      // 스포트라이트에서 사용
}; // 48byte
} // namespace Ryudar
