#include "GameFactory.h"
#include "../Configuration/Macros.h"
#include "../Engine/Engine.h"
#include "../GlobalConstants/constants.h"
#include "../Math/MathAlias.h"
#include "../Managers/GraphicsSystem.h"
#include "../Managers/LightManager.h"
#include "../Managers/ResourceManager.h"
#include "../Nodes/Node.h"
#include "../Parsing/XMLFile.h"
#include "../Scene/Scene.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../VulkanWrappers/Texture.h"
#include "../Managers/FileManager.h"
#include <assimp/color4.h>
#include <assimp/Importer.hpp>
#include <assimp/light.h>
#include <assimp/material.h>
#include <assimp/MathFunctions.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

nest::Result nest::GameFactory::LoadScene(Scene& scene, XMLFile& data)
{   
    const char* stringValue{};
    if (data.CompareRootName("Scene"))
        if (data.GetConstCharFromRoot("LoadFile", stringValue))
        {
            Assimp::Importer importer;
            const aiScene* aiScene = importer.ReadFile(stringValue, aiProcess_ConvertToLeftHanded);
            GatherAllMaterials(aiScene);
            LoadAllLights(scene, aiScene);
            CreateScene(scene, aiScene);
            return Result::kSucess;
        }
    return Result::kFailedToCreate;
}

nest::MeshNode* nest::GameFactory::CreateMeshNode(const aiMesh* mesh)
{
    return BLEACH_NEW(MeshNode(mesh));
}

void nest::GameFactory::CreateAllChildren(ParentNode* root, Scene& scene, const aiScene* aiScene, const aiNode* parent)
{
    // after creating object from parent data
    // iterate through each child to instantiate their own nodes
    for (unsigned int i = 0; i < parent->mNumChildren; ++i)
    {
        auto child = parent->mChildren[i];
        if (child->mNumMeshes > 0) // if this node contains meshes, it is a mesh node
        {
            auto* node = CreateMeshNode(aiScene->mMeshes[child->mMeshes[0]]);
            node->SetMeshData(child);
            CreateAllMeshChildren(node, scene, aiScene, child);
            root->EmplaceNode(node);
        }
        else
        {
            //if (child->mMetaData->HasKey())
            auto* node = CreateSceneNode(child);
            node->SetMeshData(child);
            CreateAllChildren(node, scene, aiScene, child);
            root->EmplaceNode(node);
        }
    }
} 

void nest::GameFactory::CreateAllMeshChildren(MeshNode* meshParent, Scene& scene, const aiScene* aiScene, const aiNode* parent)
{
    for (unsigned int i = 0; i < parent->mNumChildren; ++i)
    {
        auto child = parent->mChildren[i];
        if (child->mNumMeshes > 0)
        {
            auto* node = CreateMeshNode(aiScene->mMeshes[child->mMeshes[0]]);
            node->SetMeshData(child);
            CreateAllMeshChildren(node, scene, aiScene, child);
            meshParent->EmplaceNode(node);
            node->SetOwner(meshParent);
        }
    }
}

void nest::GameFactory::CreateScene(Scene& scene, const aiScene* aiScene)
{
    const auto& aiRootNode = aiScene->mRootNode;
    auto* sceneRoot = scene.GetRoot();
    auto* rootNode = CreateNodeFromSceneData(aiScene, aiRootNode);
    sceneRoot->EmplaceNode(rootNode);
    CreateAllChildren(rootNode, scene, aiScene, aiRootNode);
}

void nest::GameFactory::LoadAllLights([[maybe_unused]] Scene& scene, const aiScene* aiScene)
{
    if (!aiScene->HasLights())
        return;

    for (unsigned int i = 0; i < aiScene->mNumLights; ++i)
    {
        aiLight* light = aiScene->mLights[i];   
        aiNode* node = FindNodeByName(aiScene->mRootNode, light->mName.C_Str());
        if (!node)
            continue;

        LightNode* newLightNode = BLEACH_NEW(LightNode(light, node));
        scene.AddLightNode(newLightNode);
        Engine::GetManager<LightManager>()->AddLight(newLightNode);
    }
}


aiNode* nest::GameFactory::FindNodeByName(const aiNode* root, const std::string& name)
{
    if (root->mName.C_Str() == name) return const_cast<aiNode*>(root);  
    for (unsigned int i = 0; i < root->mNumChildren; ++i) 
    {
        aiNode* found = FindNodeByName(root->mChildren[i], name);
        if (found) return found;
    }
    return nullptr;
}

nest::ParentNode* nest::GameFactory::CreateNodeFromSceneData(const aiScene* aiScene, const aiNode* currentNode)
{
    if (currentNode->mNumMeshes > 0) // if this node contains meshes, it is a mesh node
    {
        // create a mesh node from the first mesh
        auto* node = CreateMeshNode(aiScene->mMeshes[currentNode->mMeshes[0]]);
        node->SetMeshData(currentNode);
        return node;
    }
    else
    {
        auto* node = CreateSceneNode(currentNode);
        node->SetMeshData(currentNode);
        return node;
    }
}

nest::SceneNode* nest::GameFactory::CreateSceneNode([[maybe_unused]] const aiNode* aiNode)
{
    return BLEACH_NEW(SceneNode());
}

//--------------------------------------------------------------------
// Helper functions

// Gathers all the materials from the loaded scene and stores them in the resource manager
void nest::GameFactory::GatherAllMaterials(const aiScene* scene)
{
    // Gathers all materials from the scene
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const auto& material = scene->mMaterials[i];
        
        aiColor4D rgba{};
        Material mater{};
        aiString filePath{};
        // check to see if a texture is associated with this mesh
        if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &filePath) == AI_SUCCESS)
        {
            mater.hasTexture = true;
            mater.textureId = nest::Engine::GetManager<nest::ResourceManager>()->AddTexture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), material);
            if (mater.textureId == -1)
                mater.hasTexture = false;
            else
            {
                auto pbrStack = nest::Engine::GetManager<nest::ResourceManager>()->GetTexture(mater.textureId);
                // isValid will set itself to false if texture was not found
                // the texture will however be a valid default texture in order to still
                // pass a descriptor set to the pipeline
                if (pbrStack->GetTexture(Material::eDiffuse)->isValid)
                    mater.textureFlags = Material::eDiffuse;
                if (pbrStack->GetTexture(Material::eNormal)->isValid)
                    mater.textureFlags += Material::eNormal;
                if (pbrStack->GetTexture(Material::eRoughness)->isValid)
                    mater.textureFlags += Material::eRoughness;
                if (pbrStack->GetTexture(Material::eAO)->isValid)
                    mater.textureFlags += Material::eAO;
                if (pbrStack->GetTexture(Material::eMetallic)->isValid)
                    mater.textureFlags += Material::eMetallic;
                if (pbrStack->GetTexture(Material::eOpacity)->isValid)
                {
                    mater.textureFlags += Material::eOpacity;
                    mater.hasTransparency = true;
                }
            }
        }
        else
        {
            // set color, if no diffuse color, use base color
            // I wasn't sure if the objects imported will always have a diffuse, so I added a condition just in-case
            if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &rgba) == AI_SUCCESS)
            {
                mater.color = { rgba.r, rgba.g, rgba.b, rgba.a };
                if (strcmp(material->GetName().C_Str(), "PlaneMaterial") == 0)
                {
                    mater.options = Material::eLit | Material::eCatchShadows;
                }
            }
            else
            {
                material->Get(AI_MATKEY_BASE_COLOR, rgba);
                mater.color = { rgba.r, rgba.g, rgba.b, rgba.a };
            }
            material->Get(AI_MATKEY_METALLIC_FACTOR, mater.metallic);
            material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mater.roughness);
            mater.hasTexture = false;
        }
        nest::Engine::GetManager<nest::ResourceManager>()->AddMaterial(i, mater);

#if _PRINT_MATERIAL_PROPERTIES == 1
        _LOG_V(LogType::kPrint, "Material ", i, " Name: ", material->GetName().C_Str(), " \n{")
        for (uint32_t p = 0; p < material->mNumProperties; ++p)
        {
            const aiMaterialProperty* prop = material->mProperties[p];
            std::string key = prop->mKey.C_Str();

            _LOG_V(LogType::kPrint, "  Property: ", key, " \n[")

            switch (prop->mType)
            {
            case aiPTI_String:
            {
                aiString str;
                memcpy(&str, prop->mData, sizeof(aiString));
                _LOG_V(LogType::kPrint, "    ", str.C_Str());
                break;
            }
            case aiPTI_Float:
            {
                const float* floatData = reinterpret_cast<const float*>(prop->mData);
                size_t count = prop->mDataLength / sizeof(float);
                for (size_t j = 0; j < count; ++j)
                    _LOG_V(LogType::kPrint, "    ", floatData[j])
                break;
            }
            case aiPTI_Integer:
            {
                const int* intData = reinterpret_cast<const int*>(prop->mData);
                size_t count = prop->mDataLength / sizeof(int);
                for (size_t j = 0; j < count; ++j)
                    _LOG_V(LogType::kPrint, "    ", intData[j])
                break;
            }
            case aiPTI_Buffer:
            {
                for (uint32_t j = 0; j < prop->mDataLength; ++j)
                    _LOG_V(LogType::kPrint, "    ", static_cast<unsigned int>(prop->mData[j]))
                break;
            }
            case aiPTI_Double:
            {
                const double* doubleData = reinterpret_cast<const double*>(prop->mData);
                size_t count = prop->mDataLength / sizeof(double);
                for (size_t j = 0; j < count; ++j)
                    _LOG_V(LogType::kPrint, "    ", doubleData[j])
                    break;
            }
            default:
                _LOG_V(LogType::kPrint, "    Unknown Property Type")
                break;
            }
            _LOG(LogType::kPrint, "]")
        }
        // add an extra endline for readability
        _LOG(LogType::kPrint, "}\n")
#endif
    }
}

// Helper functions
//--------------------------------------------------------------------