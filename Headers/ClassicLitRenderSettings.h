#pragma once
// Classic Lit 렌더링 경로의 CPU 측 설정을 정의한다.
// GUI와 프로그램 로직은 이 값을 수정하고, 렌더링 전에 GPU constant data로 변환한다.
// GPU 메모리 배치와 패딩은 ClassicLitConstantData.h에서 별도로 관리한다.

#include <directxtk/SimpleMath.h>

#include "Material.h"

namespace Ryudar::ClassicLit
{
using DirectX::SimpleMath::Vector3;

struct ShadingSettings
{
	// 표면 텍스처 사용 여부와 직접광의 specular 계산 방식을 선택한다.
	bool useTexture = false;
	bool useBlinnPhong = false;
	bool usePhong = true;
};

struct EnvironmentSettings
{
	// 환경 큐브맵을 이용한 IBL과 순수 환경 반사 사용 여부를 설정한다.
	bool useIBL = false;
	bool useEnvironmentReflection = false;
};

struct RimLightSettings
{
	// 시선과 표면 노멀의 각도를 이용해 오브젝트 외곽을 강조한다.
	Vector3 rimColor = Vector3(1.0f);
	float rimPower = 2.0f;
	float rimStrength = 0.0f;

	bool useRimLight = false;
	bool useSmoothstep = false;
};

struct RenderSettings
{
	// Classic Lit 오브젝트 하나가 유지하는 재질 및 셰이딩 설정 묶음.
	Material material;
	ShadingSettings shading;
	EnvironmentSettings environment;
	RimLightSettings rimLight;
};

} // namespace Ryudar::ClassicLit
