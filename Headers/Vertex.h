#pragma once
// 렌더링 파이프라인에서 사용하는 기본 정점 형식.
// 모델 위치, 노멀, 텍스처 좌표를 CPU/GPU 양쪽에서 동일한 순서로 사용한다.

#include <directxtk/SimpleMath.h>
#include <vector>

namespace Ryudar
{
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 texcoord;
};

} // namespace Ryudar