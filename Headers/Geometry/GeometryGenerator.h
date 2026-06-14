#pragma once
// 기본 도형 메쉬를 생성하고 외부 모델 파일을 MeshData 목록으로 변환한다.
// 렌더링 파이프라인에 넘길 CPU 측 정점과 인덱스 데이터를 만든다.

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Geometry/MeshData.h"
#include "Geometry/Vertex.h"

namespace Ryudar
{

class GeometryGenerator
{
public:
	// Assimp로 외부 모델 파일을 읽고, 모델 전체를 원점 기준 단위 크기로 정규화한다.
	static vector<MeshData> ReadFromFile(const std::string &basePath, const std::string &fileName);

	// x-y 평면에 놓인 화면/후처리용 사각형 메쉬를 만든다.
	static MeshData MakeSquare(const float scale = 1.0f);

	// 각 면이 분리된 큐브 메쉬를 만든다.
	static MeshData MakeBox(const float scale = 1.0f);

	// x-y 평면을 numSlices x numStacks 격자로 나눈 평면 메쉬를 만든다.
	static MeshData MakeGrid(const float width, const float height, const int numSlices,
	                         const int numStacks);

	// y축을 중심으로 하는 원기둥/원뿔대 메쉬를 만든다.
	static MeshData MakeCylinder(const float bottomRadius, const float topRadius, float height,
	                             int sliceCount);

	// 위도/경도 방식으로 구 메쉬를 만든다.
	static MeshData MakeSphere(const float radius, const int numSlice, const int numStacks);

	// 정사면체 테스트용 메쉬를 만든다.
	static MeshData MakeTetrahedron();

	// 정이십면체 기반 구 생성을 위한 기본 정이십면체 메쉬를 만든다.
	static MeshData MakeIcosahedron();

	// 삼각형을 세분화한 뒤 각 정점을 구 표면으로 투영한다.
	static MeshData SubdivideToSphere(const float radius, const MeshData &meshData);
};
} // namespace Ryudar
