#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>
#include <string>

#include "Vertex.h"
#include "MeshData.h"

namespace Ryudar
{

class GeometryGenerator
{
  public:
    static vector<MeshData> ReadFromFile(std::string basePath, 
                                         std::string fileName);

    static MeshData MakeSquare(const float scale = 1.0f);
    static MeshData MakeBox(const float scale = 1.0f);
    static MeshData MakeGrid(const float width, const float height, const int numSlices,
                             const int numStacks);
    static MeshData MakeCylinder(const float bottomRadius, 
                                 const float topRadius, float height,
                                 int sliceCount);
    static MeshData MakeSphere(const float radius, const int numSlice, const int numStacks);
    static MeshData MakeTetrahedron();
    static MeshData MakeIcosahedron();
    static MeshData SubdivideToSphere(const float radius, MeshData meshData);
};
}