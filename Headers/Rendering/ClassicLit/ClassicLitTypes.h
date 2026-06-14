#pragma once

// Classic Lit 렌더링 경로에서 공유하는 열거형을 정의한다.
// 값은 HLSL의 셰이딩 모델 상수와 동일하게 유지해야 한다.

#include <cstdint>

namespace Ryudar::ClassicLit
{

enum class ShadingModel : uint32_t
{
	Phong = 0,
	BlinnPhong = 1,
};

} // namespace Ryudar::ClassicLit
