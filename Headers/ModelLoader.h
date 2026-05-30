#pragma once
// Assimp를 사용해 외부 모델 파일을 읽고 프로젝트의 MeshData 형식으로 변환한다.
// 노드 계층의 transform을 누적해 각 메쉬 정점 위치에 반영한다.

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "MeshData.h"
#include "Vertex.h"

namespace Ryudar
{
class ModelLoader
{
public:
	void Load(std::string basePath, std::string fileName);

public:
	std::vector<MeshData> meshes;

private:
	void ProcessNode(aiNode *node, const aiScene *scene,
	                 DirectX::SimpleMath::Matrix parentTransform);
	MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

	std::string basePath;
};
} // namespace Ryudar
