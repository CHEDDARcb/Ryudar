#pragma once

// Classic Litレンダリングで共有するEnumを定義する。
// 値はHLSL側のShading Model定数と一致させる。

#include <cstdint>

namespace Ryudar::ClassicLit
{

enum class ShadingModel : uint32_t
{
	Phong = 0,
	BlinnPhong = 1,
};

} // namespace Ryudar::ClassicLit
