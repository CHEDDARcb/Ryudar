#include "Assets/ModelLoader.h"

#include <filesystem>

namespace Ryudar
{

using namespace DirectX::SimpleMath;

std::vector<MeshData> ModelLoader::Load(const std::string &basePath,
                                        const std::string &filename)
{
	m_basePath = basePath;
	m_meshes.clear();

	Assimp::Importer importer;

	const aiScene *pScene = importer.ReadFile(
	    this->m_basePath + filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (!pScene)
	{
		std::cout << "Failed to read file: " << this->m_basePath + filename << std::endl;
	}
	else
	{
		// 루트 노드에는 부모 변환이 없으므로 단위 행렬에서 순회를 시작한다.
		Matrix parentTransform;
		ProcessNode(pScene->mRootNode, pScene, parentTransform);
	}

	return std::move(m_meshes);
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene, Matrix parentTransform)
{
	// Assimp의 노드 변환을 DirectX 행렬로 옮기고 부모 변환을 누적한다.
	Matrix nodeTransform;
	ai_real *temp = &node->mTransformation.a1;
	float *matrixElements = &nodeTransform._11;
	for (int t = 0; t < 16; t++)
	{
		matrixElements[t] = float(temp[t]);
	}
	nodeTransform = nodeTransform.Transpose() * parentTransform;

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{

		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		auto newMesh = this->ProcessMesh(mesh, scene);

		for (auto &v : newMesh.vertices)
		{
			v.position = DirectX::SimpleMath::Vector3::Transform(v.position, nodeTransform);
		}

		m_meshes.push_back(std::move(newMesh));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransform);
	}
}

MeshData ModelLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
	MeshData newMesh;
	newMesh.vertices.reserve(mesh->mNumVertices);
	newMesh.indices.reserve(mesh->mNumFaces * 3);

	std::vector<Vertex> &vertices = newMesh.vertices;
	std::vector<uint32_t> &indices = newMesh.indices;

	// Assimp 정점 속성을 프로젝트의 공통 Vertex 형식으로 변환한다.
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		vertex.normal.Normalize();

		if (mesh->mTextureCoords[0])
		{
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// 첫 번째 디퓨즈 텍스처 파일을 메시의 재질 텍스처로 사용한다.
	// http://assimp.sourceforge.net/lib_html/materials.html
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString filepath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);

			std::string fullPath =
			    this->m_basePath +
			    std::string(std::filesystem::path(filepath.C_Str()).filename().string());

			newMesh.textureFilename = fullPath;
		}
	}

	return newMesh;
}
} // namespace Ryudar
