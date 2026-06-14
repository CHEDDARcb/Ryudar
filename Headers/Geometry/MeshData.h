#pragma once
// GPU Buffer生成前にCPUメモリ上で保持するMeshデータ構造体。
// GeometryGeneratorとModelLoaderが生成した頂点、Index、Textureパスを保持する。

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Geometry/Vertex.h"

namespace Ryudar
{

using std::vector;

struct MeshData
{
	// GPU Vertex Buffer生成前に保持する頂点一覧。
	std::vector<Vertex> vertices;

	// 頂点をTriangleまたはLineへ組み立てるためのIndex一覧。
	std::vector<uint32_t> indices;

	// このMeshが使用するDiffuse Textureパス。空の場合はTextureを生成しない。
	std::string textureFilename;
};

} // namespace Ryudar
