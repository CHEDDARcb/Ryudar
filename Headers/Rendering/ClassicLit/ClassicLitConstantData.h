#pragma once
// 通常MeshとNormal Debug描画で使用するConstant Bufferデータを定義する。
// C++構造体とHLSL Constant BufferのLayoutが一致するよう16Byte Alignmentを検証する。

#include <directxtk/SimpleMath.h>

#include "Rendering/ClassicLit/ClassicLitTypes.h"
#include "Scene/Light.h"
#include "Scene/Material.h"

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
	Light lights[MaxLights]; // 48 * 3 = 144
	Vector3 eyeWorld{};      // 12
	float padding = 0.f;     // 4
};
static_assert((sizeof(LightingConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(LightingConstantData) == 160,
              "LightingConstantData must match the HLSL cbuffer layout");

// Rim Light用のGPU定数データ。
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

// TextureとShading Modelの選択に使用するGPU定数データ。
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

// Environment設定を分離して管理し、ShadingConstantDataへ格納する。
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

// Classic Litレンダリングに必要なPixel Shading定数を一つの構造体にまとめる。
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
	float padding[3] = {};
};

static_assert((sizeof(NormalVertexConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");
static_assert(sizeof(NormalVertexConstantData) == 16,
              "NormalVertexConstantData must match the HLSL cbuffer layout");

} // namespace Ryudar
