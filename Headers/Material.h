#pragma once
// 기본 조명과 IBL 셰이딩에 사용하는 재질 데이터 구조체.
// HLSL Material 구조체와 레이아웃이 맞도록 float3 뒤에 패딩 필드를 둔다.

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "Mesh.h"

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

// 재질
struct Material
{
	Vector3 ambient = Vector3(0.0f);                 // 12
	float shininess = 10.0;                          // 4
	Vector3 diffuse = Vector3(1.0f);                 // 12
	float padding0;                                  // 4
	Vector3 specular = Vector3(0.5f);                // 12
	float padding1;                                  // 4
	Vector3 fresnelR0 = Vector3(1.0f, 0.71f, 0.29f); // Gold
	float padding2;
}; // 48byte
} // namespace Ryudar
