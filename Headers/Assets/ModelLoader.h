#pragma once
// Assimpで外部Modelファイルを読み込み、プロジェクトのMeshData形式へ変換する。
// Node階層のTransformを累積し、各Meshの頂点位置へ反映する。

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "Geometry/MeshData.h"
#include "Geometry/Vertex.h"

namespace Ryudar
{
class ModelLoader
{
public:
	// Modelファイルを読み込み、Node TransformとMaterial Textureを反映したMesh一覧を返す。
	std::vector<MeshData> Load(const std::string &basePath, const std::string &fileName);

private:
	// Node階層を走査し、親Transformを累積する。
	void ProcessNode(aiNode *node, const aiScene *scene,
	                 DirectX::SimpleMath::Matrix parentTransform);
	// Assimp Mesh一つをプロジェクトのMeshData形式へ変換する。
	MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

	std::string m_basePath;
	std::vector<MeshData> m_meshes;
};
} // namespace Ryudar
