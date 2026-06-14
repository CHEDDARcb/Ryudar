#pragma once

#include <cstdint>

namespace Ryudar::ClassicLit
{

enum class ShadingModel : uint32_t
{
	Phong = 0,
	BlinnPhong = 1,
};

} // namespace Ryudar::ClassicLit