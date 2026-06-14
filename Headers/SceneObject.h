#pragma once

#include "ClassicLitMeshGroup.h"
#include "Transform.h"

namespace Ryudar
{

struct SceneObject
{
	ClassicLit::MeshGroup meshGroup;
	Transform transform;
};

} // namespace Ryudar