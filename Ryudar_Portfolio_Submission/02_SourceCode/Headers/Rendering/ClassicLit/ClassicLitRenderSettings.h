#pragma once
// Classic Litレンダリングで使用するCPU側設定をまとめる。
// GUIで編集した後にGPU定数へ変換し、Memory Layoutは別の構造体で管理する。

#include <directxtk/SimpleMath.h>

#include "Rendering/ClassicLit/ClassicLitTypes.h"
#include "Scene/Material.h"

namespace Ryudar::ClassicLit
{
using DirectX::SimpleMath::Vector3;

struct ShadingSettings
{
	// Texture使用有無とDirect Lightの反射Modelを選択する。
	bool useTexture = false;
	ShadingModel model = ShadingModel::Phong;
};

struct EnvironmentSettings
{
	// Image Based LightingとEnvironment Reflectionの使用有無を制御する。
	bool useIBL = false;
	bool useEnvironmentReflection = false;
};

struct RimLightSettings
{
	// 視線とSurface Normalの角度を利用してObjectの輪郭を強調する。
	Vector3 rimColor = Vector3(1.0f);
	float rimPower = 2.0f;
	float rimStrength = 0.0f;

	bool useRimLight = false;
	bool useSmoothstep = false;
};

struct RenderSettings
{
	// 一つのClassic Lit Objectが独立して所有するレンダリング設定。
	Material material;
	ShadingSettings shading;
	EnvironmentSettings environment;
	RimLightSettings rimLight;
};

} // namespace Ryudar::ClassicLit
