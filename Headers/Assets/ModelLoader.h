#pragma once
// Assimp를 사용해 외부 모델 파일을 읽고 프로젝트의 MeshData 형식으로 변환한다.
// 노드 계층의 변환을 누적해 각 메시 정점 위치에 반영한다.

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
	// 모델 파일을 읽고 노드 변환과 재질 텍스처를 반영한 메시 목록을 반환한다.
	std::vector<MeshData> Load(const std::string &basePath, const std::string &fileName);

private:
	// 노드 계층을 순회하며 부모 변환을 누적한다.
	void ProcessNode(aiNode *node, const aiScene *scene,
	                 DirectX::SimpleMath::Matrix parentTransform);
	// Assimp 메시 하나를 프로젝트의 MeshData 형식으로 변환한다.
	MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

	std::string m_basePath;
	std::vector<MeshData> m_meshes;
};
} // namespace Ryudar
