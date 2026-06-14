#pragma once
// GPU 버퍼를 만들기 전에 CPU 메모리에 보관하는 메시 데이터 구조체.
// GeometryGenerator와 ModelLoader가 만든 정점/인덱스/텍스처 파일 정보를 담는다.

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Geometry/Vertex.h"

namespace Ryudar
{

using std::vector;

struct MeshData
{
	// GPU 정점 버퍼를 만들기 전에 보관하는 정점 목록.
	std::vector<Vertex> vertices;

	// 정점을 삼각형 또는 선분으로 조립할 때 사용하는 인덱스 목록.
	std::vector<uint32_t> indices;

	// 이 메시가 사용할 디퓨즈 텍스처 경로. 비어 있으면 텍스처를 만들지 않는다.
	std::string textureFilename;
};

} // namespace Ryudar
