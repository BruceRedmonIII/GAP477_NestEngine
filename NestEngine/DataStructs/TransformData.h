#pragma once
#include "../Uniforms/MeshDataStructs.h"
#include "../Math/MathAlias.h"
#include <string>
namespace nest
{
    struct MeshData
    {
        Material material{};
        std::string name;
        Mat4 transform = glm::identity<Mat4>();
    };
}