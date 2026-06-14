#pragma once
// Classic Lit 렌더링 경로에서 사용하는 CPU 측 설정을 모아둔다.
// GUI에서 수정한 뒤 GPU 상수 데이터로 변환하며, 메모리 레이아웃은 별도 구조체가 담당한다.

#include <directxtk/SimpleMath.h>

#include "Rendering/ClassicLit/ClassicLitTypes.h"
#include "Scene/Material.h"

namespace Ryudar::ClassicLit
{
using DirectX::SimpleMath::Vector3;

struct ShadingSettings
{
	// 텍스처 사용 여부와 직접광의 반사 모델을 선택한다.
	bool useTexture = false;
	ShadingModel model = ShadingModel::Phong;
};

struct EnvironmentSettings
{
	// 이미지 기반 조명과 환경 반사 사용 여부를 제어한다.
	bool useIBL = false;
	bool useEnvironmentReflection = false;
};

struct RimLightSettings
{
	// 시선과 표면 노멀의 각도를 이용해 객체 가장자리를 강조한다.
	Vector3 rimColor = Vector3(1.0f);
	float rimPower = 2.0f;
	float rimStrength = 0.0f;

	bool useRimLight = false;
	bool useSmoothstep = false;
};

struct RenderSettings
{
	// 하나의 Classic Lit 객체가 독립적으로 소유하는 렌더링 설정이다.
	Material material;
	ShadingSettings shading;
	EnvironmentSettings environment;
	RimLightSettings rimLight;
};

} // namespace Ryudar::ClassicLit
