#pragma once
// CPU-side 메쉬 데이터 컨테이너.
// GeometryGenerator와 ModelLoader가 만든 정점/인덱스/텍스처 파일 정보를 담는다.

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Vertex.h"

namespace Ryudar
{

using std::vector;

struct MeshData
{
	// GPU vertex buffer를 만들기 전 CPU 메모리에 보관하는 정점 목록.
	std::vector<Vertex> vertices;

	// vertices를 삼각형 또는 선분으로 조립할 때 사용하는 인덱스 목록.
	std::vector<uint32_t> indices;

	// 이 메쉬가 사용할 diffuse texture 파일 경로. 비어 있으면 텍스처를 만들지 않는다.
	std::string textureFilename;
};

} // namespace Ryudar
