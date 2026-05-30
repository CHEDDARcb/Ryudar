#include "GeometryGenerator.h"

#include "ModelLoader.h"

namespace Ryudar
{
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshData GeometryGenerator::MakeSquare(const float scale)
{
	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector3> normals;
	vector<Vector2> texcoords; // 텍스춰 좌표

	// 앞면
	positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

	// TODO: 텍스춰 좌표 추가
	// Texture Coordinates (Direct3D 9)
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	MeshData meshData;
	for (size_t i = 0; i < positions.size(); i++)
	{
		Vertex v;
		v.position = positions[i];
		v.normal = normals[i];
		v.texcoord = texcoords[i];

		meshData.vertices.push_back(v);
	}
	meshData.indices = {
	    0, 1, 2, 0, 2, 3, // 앞면
	};

	return meshData;
}

MeshData GeometryGenerator::MakeBox(const float scale)
{

	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector3> normals;
	vector<Vector2> texcoords;

	// 윗면
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// 아랫면
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// 앞면
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// 뒷면
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// 왼쪽
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// 오른쪽
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	MeshData meshData;
	for (size_t i = 0; i < positions.size(); i++)
	{
		Vertex v;
		v.position = positions[i];
		v.normal = normals[i];
		v.texcoord = texcoords[i];
		meshData.vertices.push_back(v);
	}

	meshData.indices = {
	    0,  1,  2,  0,  2,  3,  // 윗면
	    4,  5,  6,  4,  6,  7,  // 아랫면
	    8,  9,  10, 8,  10, 11, // 앞면
	    12, 13, 14, 12, 14, 15, // 뒷면
	    16, 17, 18, 16, 18, 19, // 왼쪽
	    20, 21, 22, 20, 22, 23  // 오른쪽
	};

	return meshData;
}

MeshData GeometryGenerator::MakeGrid(const float width, const float height, const int numSlices,
                                     const int numStacks)
{
	// x-y 평면 (z = 0) 위에 격자 구조로 평면 만들기
	// 뒤에서 복잡한 지형으로 확장

	// 1단계: numStacks = 1 이고 numSlices만 고려해서 구현
	// 2단계: 2차원 바둑판 구조

	const float dx = width / numSlices;
	const float dy = height / numStacks;

	MeshData meshData;

	vector<Vertex> &vertices = meshData.vertices;
	vector<uint32_t> &indices = meshData.indices;

	Vector3 leftBottom = Vector3(-0.5f * width, -0.5f * height, 0.0f);
	for (int j = 0; j <= numStacks; j++)
	{
		// x-y평면에서 시작점을 y방향을 이동
		Vector3 stackStartPoint =
		    Vector3::Transform(leftBottom, Matrix::CreateTranslation(0.0f, dy * j, 0.0f));
		for (int i = 0; i <= numSlices; i++)
		{
			Vertex v;
			// x-y평면에서 시작점을 x 방향을 이동
			v.position =
			    Vector3::Transform(stackStartPoint, Matrix::CreateTranslation(dx * i, 0.0f, 0.0f));
			v.normal = Vector3(0.0f, 0.0f, -1.0f); // 시점을 향하는 방향
			v.texcoord = Vector2(1.0f / numSlices * i, 1.0f - (1.0f / numStacks) * j);

			vertices.push_back(v);
		}
	}

	// 인덱스 추가
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
	// Texture 좌표계때문에 (sliceCount + 1) x 2 개의 버텍스 사용
	// -> Texture좌표계가 0에서 시작해서 1로 와야함.
	// -> 마지막 Texture좌표계를 처리해줄때,
	// 1자리에 맨 처음 처리한 0좌표계를 추가로 처리함.

	const float dTheta = -XM_2PI / float(sliceCount);

	MeshData meshData;

	vector<Vertex> &vertices = meshData.vertices;
	// 옆면의 바닥 버텍스들 (인덱스 0 이상 sliceCount 미만)
	for (int i = 0; i <= sliceCount; i++)
	{
		Vertex v;

		v.position = Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
		                                Matrix::CreateRotationY(dTheta * float(i)));
		// vertex의 위치로부터 원의 중심을 빼주면 Normal벡터.
		v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
		v.normal.Normalize();
		v.texcoord = Vector2(0.0f + 1.0f / sliceCount * float(i), 1.0f);

		vertices.push_back(v);
	}

	// 옆면의 맨 위 버텍스들 (인덱스 sliceCount 이상 2 * sliceCount 미만)
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

	// index추가
	vector<uint32_t> &indices = meshData.indices;
	for (int i = 0; i < sliceCount; i++)
	{
		// 첫번째 삼각형
		indices.push_back(i);
		indices.push_back(i + sliceCount + 1);
		indices.push_back(i + sliceCount + 1 + 1);

		// 두번째 삼각형
		indices.push_back(i);
		indices.push_back(i + sliceCount + 1 + 1);
		indices.push_back(i + 1);
	}

	return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices, const int numStacks)
{
	// 참고: OpenGL Sphere
	// http://www.songho.ca/opengl/gl_sphere.html
	// Texture 좌표계때문에 (numSlices + 1) 개의 버텍스 사용 (마지막에 닫아주는
	// 버텍스가 중복) Stack은 y 위쪽 방향으로 쌓아가는 방식

	const float dTheta = -XM_2PI / float(numSlices);
	const float dPhi = -XM_PI / float(numStacks);

	MeshData meshData;

	vector<Vertex> &vertices = meshData.vertices;
	for (int j = 0; j <= numStacks; j++)
	{
		// 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
		Vector3 stackStartPoint =
		    Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));
		for (int i = 0; i <= numSlices; i++)
		{
			Vertex v;

			// 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
			v.position =
			    Vector3::Transform(stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));
			v.normal = v.position; // 원점이 구의 중심(0, 0, 0).
			v.normal.Normalize();
			v.texcoord = Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

			vertices.push_back(v);
		}
	}

	// index 추가
	vector<uint32_t> &indices = meshData.indices;
	for (int j = 0; j < numStacks; j++)
	{
		const int offset = (numSlices + 1) * j;

		for (int i = 0; i < numSlices; i++)
		{
			// 첫번째 삼각형
			indices.push_back(offset + i);
			indices.push_back(offset + i + numSlices + 1);
			indices.push_back(offset + i + numSlices + 1 + 1);

			// 두번쨰 삼각형
			indices.push_back(offset + i);
			indices.push_back(offset + i + numSlices + 1 + 1);
			indices.push_back(offset + i + 1);
		}
	}

	return meshData;
}

MeshData GeometryGenerator::MakeIcosahedron()
{
	// Luna DX12 교재 참고
	// 등20면체
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

	// Regular Tetrahedron
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
		v.normal = v.position; // 중심이 원점
		v.normal.Normalize();

		meshData.vertices.push_back(v);
	}

	meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};

	return meshData;
}

MeshData GeometryGenerator::SubdivideToSphere(const float radius, MeshData meshData)
{
	auto ProjectVertex = [&](Vertex &v)
	{
		v.normal = v.position;
		v.normal.Normalize();
		v.position = v.normal * radius;

		// 주의: 텍스춰가 이음매에서 깨짐
		// atan vs atan2
		// https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
		const float theta = atan2f(v.position.z, v.position.x);
		const float phi = acosf(v.position.y / radius);
		v.texcoord.x = theta / XM_2PI;
		v.texcoord.y = phi / XM_PI;
	};

	auto UpdateFaceNormal = [](Vertex &v0, Vertex &v1, Vertex &v2)
	{
		// v0, v1, v2로 이루어진 삼각형의 faceNoraml 계산
		auto faceNormal = (v1.position - v0.position).Cross(v2.position - v0.position);
		faceNormal.Normalize();
		v0.normal = faceNormal;
		v1.normal = faceNormal;
		v2.normal = faceNormal;
	};

	// 버텍스가 중복되는 구조로 구현
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
		// 위치와 텍스처 좌표 결정
		v3.position = (v0.position + v2.position) * 0.5f;
		v3.texcoord = (v0.texcoord + v2.texcoord) * 0.5f;
		ProjectVertex(v3);

		Vertex v4;
		// 위치와 텍스처 좌표 결정
		v4.position = (v0.position + v1.position) * 0.5f;
		v4.texcoord = (v0.texcoord + v1.texcoord) * 0.5f;
		ProjectVertex(v4);

		Vertex v5;
		// 위치와 텍스처 좌표 결정
		v5.position = (v1.position + v2.position) * 0.5f;
		v5.texcoord = (v1.texcoord + v2.texcoord) * 0.5f;
		ProjectVertex(v5);

		// UpdateFaceNormal(v4, v1, v5);
		// UpdateFaceNormal(v0, v4, v3);
		// UpdateFaceNormal(v3, v4, v5);
		// UpdateFaceNormal(v3, v5, v2);

		// 모든 버텍스 새로 추가
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

vector<MeshData> GeometryGenerator::ReadFromFile(std::string basePath, std::string filename)
{

	using namespace DirectX;

	ModelLoader modelLoader;
	modelLoader.Load(basePath, filename);
	vector<MeshData> &meshes = modelLoader.meshes;

	// Normalize vertices
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
	float cx = (maxBounds.x + minBounds.x) * 0.5f,
	      cy = (maxBounds.y + minBounds.y) * 0.5f,
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
