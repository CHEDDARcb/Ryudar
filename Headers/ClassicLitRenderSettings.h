#pragma once
// CPU-side settings for the Classic Lit rendering path.
// The GUI and application logic edit these values before they are converted to GPU constants.
// GPU memory layout and padding are managed separately in ClassicLitConstantData.h.

#include <directxtk/SimpleMath.h>

#include "ClassicLitTypes.h"
#include "Material.h"

namespace Ryudar::ClassicLit
{
using DirectX::SimpleMath::Vector3;

struct ShadingSettings
{
	// Selects texture use and the specular model used for direct lighting.
	bool useTexture = false;
	ShadingModel model = ShadingModel::Phong;
};

struct EnvironmentSettings
{
	// Controls image-based lighting and direct environment reflection.
	bool useIBL = false;
	bool useEnvironmentReflection = false;
};

struct RimLightSettings
{
	// Highlights object edges using the angle between the view and surface normal.
	Vector3 rimColor = Vector3(1.0f);
	float rimPower = 2.0f;
	float rimStrength = 0.0f;

	bool useRimLight = false;
	bool useSmoothstep = false;
};

struct RenderSettings
{
	// Material and shading settings owned by one Classic Lit object.
	Material material;
	ShadingSettings shading;
	EnvironmentSettings environment;
	RimLightSettings rimLight;
};

} // namespace Ryudar::ClassicLit
