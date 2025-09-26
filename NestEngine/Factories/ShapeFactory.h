#pragma once
#include "Factory.h"
#include "../Math/MathAlias.h"
template <class HasId>
concept HashedAbstract = requires
{
    HasId::kHashedId;
};
struct aiMesh;
struct aiScene;
struct aiNode;
namespace nest
{
    class Scene;
    class SceneNode;
    class BaseNode;
    class XMLFile;
    class ParentNode;
    class MeshNode;
    class ShapeFactory : public Factory
    {
    public:
        SET_HASHED_ID(ShapeFactory)
        ShapeFactory() = default;
        Result LoadCube(MeshNode* mesh);
    };
}