#include "Geometry/GeometryGenerator.h"

#include "Assets/ModelLoader.h"

namespace Ryudar
{
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshData GeometryGenerator::MakeSquare(const float scale)
{
	MeshData meshData;
	const Vector3 normal(0.0f, 0.0f, -1.0f);

	meshData.vertices = {
	    {Vector3(-1.0f, 1.0f, 0.0f) * scale, normal, Vector2(0.0f, 0.0f)},
	    {Vector3(1.0f, 1.0f, 0.0f) * scale, normal, Vector2(1.0f, 0.0f)},
	    {Vector3(1.0f, -1.0f, 0.0f) * scale, normal, Vector2(1.0f, 1.0f)},
	    {Vector3(-1.0f, -1.0f, 0.0f) * scale, normal, Vector2(0.0f, 1.0f)},
	};
	meshData.indices = {
	    0, 1, 2, 0, 2, 3, // 前面
	};

	return meshData;
}

MeshData GeometryGenerator::MakeBox(const float scale)
{
	MeshData meshData;
	meshData.vertices.reserve(24);

	auto AddFace = [&](const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
	                   const Vector3 &normal)
	{
		meshData.vertices.push_back({p0 * scale, normal, Vector2(0.0f, 0.0f)});
		meshData.vertices.push_back({p1 * scale, normal, Vector2(1.0f, 0.0f)});
		meshData.vertices.push_back({p2 * scale, normal, Vector2(1.0f, 1.0f)});
		meshData.vertices.push_back({p3 * scale, normal, Vector2(0.0f, 1.0f)});
	};

	// 上面
	AddFace(Vector3(-1.0f, 1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f),
	        Vector3(1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f));
	// 下面
	AddFace(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, 1.0f),
	        Vector3(-1.0f, -1.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f));
	// 前面
	AddFace(Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f),
	        Vector3(1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f));
	// 背面
	AddFace(Vector3(-1.0f, -1.0f, 1.0f), Vector3(1.0f, -1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f),
	        Vector3(-1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f));
	// 左面
	AddFace(Vector3(-1.0f, -1.0f, 1.0f), Vector3(-1.0f, 1.0f, 1.0f), Vector3(-1.0f, 1.0f, -1.0f),
	        Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f));
	// 右面
	AddFace(Vector3(1.0f, -1.0f, 1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f),
	        Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f));

	meshData.indices = {
	    0,  1,  2,  0,  2,  3,  // 上面
	    4,  5,  6,  4,  6,  7,  // 下面
	    8,  9,  10, 8,  10, 11, // 前面
	    12, 13, 14, 12, 14, 15, // 背面
	    16, 17, 18, 16, 18, 19, // 左面
	    20, 21, 22, 20, 22, 23  // 右面
	};

	return meshData;
}

MeshData GeometryGenerator::MakeGrid(const float width, const float height, const int numSlices,
                                     const int numStacks)
{
	// x-y平面を行と列に分割し、各Cellを二つのTriangleで構成する。

	const float dx = width / numSlices;
	const float dy = height / numStacks;

	MeshData meshData;
	meshData.vertices.reserve((numSlices + 1) * (numStacks + 1));
	meshData.indices.reserve(numSlices * numStacks * 6);

	vector<Vertex> &vertices = meshData.vertices;
	vector<uint32_t> &indices = meshData.indices;

	Vector3 leftBottom = Vector3(-0.5f * width, -0.5f * height, 0.0f);
	for (int j = 0; j <= numStacks; j++)
	{
		// x-y平面上の開始点をy方向へ移動する。
		Vector3 stackStartPoint =
		    Vector3::Transform(leftBottom, Matrix::CreateTranslation(0.0f, dy * j, 0.0f));
		for (int i = 0; i <= numSlices; i++)
		{
			Vertex v;
			// 各行の開始点をx方向へ移動する。
			v.position =
			    Vector3::Transform(stackStartPoint, Matrix::CreateTranslation(dx * i, 0.0f, 0.0f));
			v.normal = Vector3(0.0f, 0.0f, -1.0f); // 視点方向を向くNormal
			v.texcoord = Vector2(1.0f / numSlices * i, 1.0f - (1.0f / numStacks) * j);

			vertices.push_back(v);
		}
	}

	for (int j = 0; j < numStacks; j++)
	{
		for (int i = 0; i < numSlices; i++)
		{
			indices.push_back(i + (numSlices + 1) * j);
			indices.push_back(i + (numSlices + 1) * (j + 1));
			indices.push_back(i + (numSlices + 1) * (j + 1) + 1);

			indices.push_back(i + (numSlices + 1) * j);
			indices.push_back(i + (numSlices + 1) * (j + 1) + 1);
			indices.push_back(i + (numSlices + 1) * j + 1);
		}
	}

	return meshData;
}

MeshData GeometryGenerator::MakeCylinder(const float bottomRadius, const float topRadius,
                                         float height, int sliceCount)
{
	// Texture座標のSeamを閉じるため、開始頂点を末尾にもう一度生成する。

	const float dTheta = -XM_2PI / float(sliceCount);

	MeshData meshData;
	meshData.vertices.reserve((sliceCount + 1) * 2);
	meshData.indices.reserve(sliceCount * 6);

	vector<Vertex> &vertices = meshData.vertices;
	// 下側と上側の頂点Ringをそれぞれ生成する。
	for (int i = 0; i <= sliceCount; i++)
	{
		Vertex v;

		v.position = Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
		                                Matrix::CreateRotationY(dTheta * float(i)));
		// 円の中心から頂点へ向かう方向を側面Normalとして使用する。
		v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
		v.normal.Normalize();
		v.texcoord = Vector2(0.0f + 1.0f / sliceCount * float(i), 1.0f);

		vertices.push_back(v);
	}

	for (int i = 0; i <= sliceCount; i++)
	{
		Vertex v;
		v.position = Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f),
		                                Matrix::CreateRotationY(dTheta * float(i)));
		v.normal = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
		v.normal.Normalize();
		v.texcoord = Vector2(0.0f + 1.0f / sliceCount * float(i), 0.0f);

		vertices.push_back(v);
	}

	vector<uint32_t> &indices = meshData.indices;
	for (int i = 0; i < sliceCount; i++)
	{
		indices.push_back(i);
		indices.push_back(i + sliceCount + 1);
		indices.push_back(i + sliceCount + 1 + 1);

		indices.push_back(i);
		indices.push_back(i + sliceCount + 1 + 1);
		indices.push_back(i + 1);
	}

	return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices, const int numStacks)
{
	// 緯度・経度方向に頂点を生成する。
	// http://www.songho.ca/opengl/gl_sphere.html
	// Texture座標のSeamを閉じるため、各Stackの開始頂点を末尾にもう一度生成する。

	const float dTheta = -XM_2PI / float(numSlices);
	const float dPhi = -XM_PI / float(numStacks);

	MeshData meshData;
	meshData.vertices.reserve((numSlices + 1) * (numStacks + 1));
	meshData.indices.reserve(numSlices * numStacks * 6);

	vector<Vertex> &vertices = meshData.vertices;
	for (int j = 0; j <= numStacks; j++)
	{
		// Stackごとに開始点をx-y平面で回転させ、緯度方向へ移動する。
		Vector3 stackStartPoint =
		    Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));
		for (int i = 0; i <= numSlices; i++)
		{
			Vertex v;

			// 開始点をx-z平面で回転させ、経度方向の円を生成する。
			v.position =
			    Vector3::Transform(stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));
			v.normal = v.position; // Sphereの中心は原点(0, 0, 0)。
			v.normal.Normalize();
			v.texcoord = Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

			vertices.push_back(v);
		}
	}

	vector<uint32_t> &indices = meshData.indices;
	for (int j = 0; j < numStacks; j++)
	{
		const int offset = (numSlices + 1) * j;

		for (int i = 0; i < numSlices; i++)
		{
			indices.push_back(offset + i);
			indices.push_back(offset + i + numSlices + 1);
			indices.push_back(offset + i + numSlices + 1 + 1);

			indices.push_back(offset + i);
			indices.push_back(offset + i + numSlices + 1 + 1);
			indices.push_back(offset + i + 1);
		}
	}

	return meshData;
}

MeshData GeometryGenerator::MakeIcosahedron()
{
	// 正二十面体の頂点とTriangle Indexを直接構成する。
	// https://mathworld.wolfram.com/Isohedron.html

	const float X = 0.525731f;
	const float Z = 0.850651f;

	MeshData newMesh;

	vector<Vector3> pos = {Vector3(-X, 0.0f, Z), Vector3(X, 0.0f, Z),   Vector3(-X, 0.0f, -Z),
	                       Vector3(X, 0.0f, -Z), Vector3(0.0f, Z, X),   Vector3(0.0f, Z, -X),
	                       Vector3(0.0f, -Z, X), Vector3(0.0f, -Z, -X), Vector3(Z, X, 0.0f),
	                       Vector3(-Z, X, 0.0f), Vector3(Z, -X, 0.0f),  Vector3(-Z, -X, 0.0f)};

	for (size_t i = 0; i < pos.size(); i++)
	{
		Vertex v;
		v.position = pos[i];
		v.normal = v.position;
		v.normal.Normalize();

		newMesh.vertices.push_back(v);
	}

	newMesh.indices = {1, 4,  0, 4, 9, 0,  4, 5, 9,  8, 5, 4,  1,  8,  4, 1, 10, 8,  10, 3,
	                   8, 8,  3, 5, 3, 2,  5, 3, 7,  2, 3, 10, 7,  10, 6, 7, 6,  11, 7,  6,
	                   0, 11, 6, 1, 0, 10, 1, 6, 11, 0, 9, 2,  11, 9,  5, 2, 9,  11, 2,  7};

	return newMesh;
}

MeshData GeometryGenerator::MakeTetrahedron()
{

	// 正四面体を原点中心へ移動して生成する。
	// https://mathworld.wolfram.com/RegularTetrahedron.html

	const float a = 1.0f;
	const float x = sqrt(3.0f) / 3.0f * a;
	const float d = sqrt(3.0f) / 6.0f * a; // = x / 2
	const float h = sqrt(6.0f) / 3.0f * a;

	vector<Vector3> points = {
	    {0.0f, x, 0.0f}, {-0.5f * a, -d, 0.0f}, {+0.5f * a, -d, 0.0f}, {0.0f, 0.0f, h}};

	Vector3 center = Vector3(0.0f);

	for (int i = 0; i < 4; i++)
	{
		center += points[i];
	}
	center /= 4.0f;

	for (int i = 0; i < 4; i++)
	{
		points[i] -= center;
	}

	MeshData meshData;

	for (int i = 0; i < points.size(); i++)
	{

		Vertex v;
		v.position = points[i];
		v.normal = v.position; // 中心は原点
		v.normal.Normalize();

		meshData.vertices.push_back(v);
	}

	meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};

	return meshData;
}

MeshData GeometryGenerator::SubdivideToSphere(const float radius, const MeshData &meshData)
{
	auto ProjectVertex = [&](Vertex &v)
	{
		v.normal = v.position;
		v.normal.Normalize();
		v.position = v.normal * radius;

		// 球面座標をUVへ変換するため、経度Seamで補正が必要になる場合がある。
		// https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
		const float theta = atan2f(v.position.z, v.position.x);
		const float phi = acosf(v.position.y / radius);
		v.texcoord.x = theta / XM_2PI;
		v.texcoord.y = phi / XM_PI;
	};

	auto UpdateFaceNormal = [](Vertex &v0, Vertex &v1, Vertex &v2)
	{
		// Triangleの三頂点へ同じFace Normalを適用する。
		auto faceNormal = (v1.position - v0.position).Cross(v2.position - v0.position);
		faceNormal.Normalize();
		v0.normal = faceNormal;
		v1.normal = faceNormal;
		v2.normal = faceNormal;
	};

	// 頂点を共有せず、Triangleごとに複製する構造。
	MeshData newMesh;
	uint16_t count = 0;
	for (size_t i = 0; i < meshData.indices.size(); i += 3)
	{
		size_t i0 = meshData.indices[i];
		size_t i1 = meshData.indices[i + 1];
		size_t i2 = meshData.indices[i + 2];

		Vertex v0 = meshData.vertices[i0];
		Vertex v1 = meshData.vertices[i1];
		Vertex v2 = meshData.vertices[i2];

		ProjectVertex(v0);
		ProjectVertex(v1);
		ProjectVertex(v2);

		Vertex v3;
		// 位置とTexture座標を計算する。
		v3.position = (v0.position + v2.position) * 0.5f;
		v3.texcoord = (v0.texcoord + v2.texcoord) * 0.5f;
		ProjectVertex(v3);

		Vertex v4;
		// 位置とTexture座標を計算する。
		v4.position = (v0.position + v1.position) * 0.5f;
		v4.texcoord = (v0.texcoord + v1.texcoord) * 0.5f;
		ProjectVertex(v4);

		Vertex v5;
		// 位置とTexture座標を計算する。
		v5.position = (v1.position + v2.position) * 0.5f;
		v5.texcoord = (v1.texcoord + v2.texcoord) * 0.5f;
		ProjectVertex(v5);

		// 細分化した四つのTriangleを新しいMeshへ追加する。
		newMesh.vertices.push_back(v4);
		newMesh.vertices.push_back(v1);
		newMesh.vertices.push_back(v5);

		newMesh.vertices.push_back(v0);
		newMesh.vertices.push_back(v4);
		newMesh.vertices.push_back(v3);

		newMesh.vertices.push_back(v3);
		newMesh.vertices.push_back(v4);
		newMesh.vertices.push_back(v5);

		newMesh.vertices.push_back(v3);
		newMesh.vertices.push_back(v5);
		newMesh.vertices.push_back(v2);

		for (uint16_t j = 0; j < 12; j++)
		{
			newMesh.indices.push_back(j + count);
		}
		count += 12;
	}

	return newMesh;
}

vector<MeshData> GeometryGenerator::ReadFromFile(const std::string &basePath,
                                                 const std::string &filename)
{

	using namespace DirectX;

	ModelLoader modelLoader;
	vector<MeshData> meshes = modelLoader.Load(basePath, filename);

	// Model全体が原点基準の単位サイズに収まるよう頂点位置を正規化する。
	Vector3 minBounds(1000, 1000, 1000);
	Vector3 maxBounds(-1000, -1000, -1000);
	for (auto &mesh : meshes)
	{
		for (auto &v : mesh.vertices)
		{
			minBounds.x = XMMin(minBounds.x, v.position.x);
			minBounds.y = XMMin(minBounds.y, v.position.y);
			minBounds.z = XMMin(minBounds.z, v.position.z);
			maxBounds.x = XMMax(maxBounds.x, v.position.x);
			maxBounds.y = XMMax(maxBounds.y, v.position.y);
			maxBounds.z = XMMax(maxBounds.z, v.position.z);
		}
	}

	float dx = maxBounds.x - minBounds.x, dy = maxBounds.y - minBounds.y,
	      dz = maxBounds.z - minBounds.z;
	float maxExtent = XMMax(XMMax(dx, dy), dz);
	float cx = (maxBounds.x + minBounds.x) * 0.5f, cy = (maxBounds.y + minBounds.y) * 0.5f,
	      cz = (maxBounds.z + minBounds.z) * 0.5f;

	for (auto &mesh : meshes)
	{
		for (auto &v : mesh.vertices)
		{
			v.position.x = (v.position.x - cx) / maxExtent;
			v.position.y = (v.position.y - cy) / maxExtent;
			v.position.z = (v.position.z - cz) / maxExtent;
		}
	}

	return meshes;
}

} // namespace Ryudar
