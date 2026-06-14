#pragma once

// 一つのScene Objectが所有するレンダリングリソースとWorld Transformをまとめる。
// Objectごとに独立したTransformを持ち、GUI編集結果が互いに干渉しないようにする。

#include "Rendering/ClassicLit/ClassicLitMeshGroup.h"
#include "Scene/Transform.h"

namespace Ryudar
{

struct SceneObject
{
	ClassicLit::MeshGroup meshGroup;
	Transform transform;
};

} // namespace Ryudar
