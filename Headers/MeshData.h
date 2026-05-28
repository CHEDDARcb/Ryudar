#pragma once

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Vertex.h"

namespace Ryudar {

using std::vector;

struct MeshData{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices; // uint32로 변경
    std::string textureFilename;
};

} // namespace hlab