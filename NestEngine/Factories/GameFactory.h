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
    class GameFactory : public Factory
    {
    public:
        SET_HASHED_ID(GameFactory)
        GameFactory() = default;
        Result LoadScene(Scene& scene, XMLFile& data); // this takes a file as its going to parse the entirety
    private:
        nest::SceneNode* CreateSceneNode(const aiNode* aiNode);
        nest::MeshNode* CreateMeshNode(const aiMesh* mesh);
        void CreateAllChildren(ParentNode* root, Scene& scene, const aiScene* aiScene, const aiNode* parent);
        void CreateAllMeshChildren(MeshNode* meshParent, Scene& scene, const aiScene* aiScene, const aiNode* parent);
        void CreateScene(Scene& scene, const aiScene* aiScene);
        void LoadAllLights(Scene& scene, const aiScene* aiScene);
        aiNode* FindNodeByName(const aiNode* root, const std::string& name);
        nest::ParentNode* CreateNodeFromSceneData(const aiScene* aiScene, const aiNode* parent);
        // helpers
        void GatherAllMaterials(const aiScene* scene);
    };
}