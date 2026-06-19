#pragma once
// 基本形状のMeshを生成し、外部ModelファイルをMeshData一覧へ変換する。
// Rendering Pipelineへ渡すCPU側の頂点・Indexデータを生成する。

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
	// Assimpで外部Modelを読み込み、Model全体を原点基準の単位サイズに正規化する。
	static vector<MeshData> ReadFromFile(const std::string &basePath, const std::string &fileName);

	// x-y平面上に画面表示/Post Process用の四角形Meshを生成する。
	static MeshData MakeSquare(const float scale = 1.0f);

	// 各面が分離されたCube Meshを生成する。
	static MeshData MakeBox(const float scale = 1.0f);

	// x-y平面をnumSlices x numStacksのGridに分割したMeshを生成する。
	static MeshData MakeGrid(const float width, const float height, const int numSlices,
	                         const int numStacks);

	// y軸を中心とするCylinder/Truncated Cone Meshを生成する。
	static MeshData MakeCylinder(const float bottomRadius, const float topRadius, float height,
	                             int sliceCount);

	// 緯度・経度方式でSphere Meshを生成する。
	static MeshData MakeSphere(const float radius, const int numSlice, const int numStacks);

	// テスト用の正四面体Meshを生成する。
	static MeshData MakeTetrahedron();

	// IcosahedronベースのSphere生成に使う正二十面体Meshを生成する。
	static MeshData MakeIcosahedron();

	// Triangleを細分化し、各頂点をSphere表面へ投影する。
	static MeshData SubdivideToSphere(const float radius, const MeshData &meshData);
};
} // namespace Ryudar
