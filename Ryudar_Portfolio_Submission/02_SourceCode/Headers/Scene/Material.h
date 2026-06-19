#pragma once
// Direct LightとIBL Shadingで使用するMaterialデータ構造体。
// HLSL側のMaterial構造体とLayoutを合わせるため、float3の後にPaddingを配置する。

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "Geometry/Mesh.h"

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

// Material
struct Material
{
	Vector3 ambient = Vector3(0.0f);                 // 12
	float shininess = 10.0;                          // 4
	Vector3 diffuse = Vector3(1.0f);                 // 12
	float padding0 = 0.0f;                           // 4
	Vector3 specular = Vector3(0.5f);                // 12
	float padding1 = 0.0f;                           // 4
	Vector3 fresnelR0 = Vector3(1.0f, 0.71f, 0.29f); // 金の基本反射率
	float padding2 = 0.0f;
}; // 64byte
} // namespace Ryudar
