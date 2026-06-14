#pragma once
// Rendering Pipelineで使用する基本頂点形式。
// Model位置、Normal、Texture座標をCPU/GPU双方で同じ順序に保つ。

#include <directxtk/SimpleMath.h>
#include <vector>

namespace Ryudar
{
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

struct Vertex
{
	Vector3 position = Vector3(0.0f);
	Vector3 normal = Vector3(0.0f);
	Vector2 texcoord = Vector2(0.0f);
};

} // namespace Ryudar
