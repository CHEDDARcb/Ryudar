#pragma once

// 하나의 씬 객체가 소유하는 렌더링 리소스와 월드 변환을 묶는다.
// 객체마다 독립적인 Transform을 가져 GUI 편집 결과가 서로 섞이지 않게 한다.

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
