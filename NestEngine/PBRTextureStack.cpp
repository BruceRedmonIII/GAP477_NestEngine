#include "PBRTextureStack.h"
#include <assimp/material.h>
#include "Managers/FileManager.h"
#include "GlobalConstants/constants.h"
#include "lib/BleachNew/BleachNew.h"
nest::PBRTextureStack::PBRTextureStack(const std::string& path, aiMaterial* material)
{
    aiString filePath{};
    ResolveDiffuse(material);
    ResolveNormal(material);
    ResolveRoughness(material);
    ResolveMetallic(material);
    ResolveAO(material);
    ResolveOpacity(material);
}

nest::PBRTextureStack::~PBRTextureStack()
{
    BLEACH_DELETE(m_diffuse);
    BLEACH_DELETE(m_roughness);
    BLEACH_DELETE(m_ambientOcclusion);
    BLEACH_DELETE(m_metallic);
    BLEACH_DELETE(m_normal);
    BLEACH_DELETE(m_opacity);
}

void nest::PBRTextureStack::ResolveDiffuse(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &filePath) == AI_SUCCESS)
        m_diffuse = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eDiffuse));
    else
        m_diffuse = BLEACH_NEW(Texture("", Material::eDiffuse));

    if (m_diffuse->isValid)
        m_isValid = true;
}

void nest::PBRTextureStack::ResolveNormal(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &filePath) == AI_SUCCESS)
        m_normal = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eNormal));
    else
        m_normal = BLEACH_NEW(Texture("", Material::eNormal));
}

void nest::PBRTextureStack::ResolveRoughness(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &filePath) == AI_SUCCESS)
        m_roughness = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eRoughness));
    else
        m_roughness = BLEACH_NEW(Texture("", Material::eRoughness));
}

void nest::PBRTextureStack::ResolveMetallic(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_METALNESS, 0, &filePath) == AI_SUCCESS)
        m_metallic = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eMetallic));
    else
        m_metallic = BLEACH_NEW(Texture("", Material::eMetallic));
}

void nest::PBRTextureStack::ResolveAO(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_LIGHTMAP, 0, &filePath) == AI_SUCCESS)
        m_ambientOcclusion = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eAO));
    else
        m_ambientOcclusion = BLEACH_NEW(Texture("", Material::eAO));
}

void nest::PBRTextureStack::ResolveOpacity(aiMaterial* material)
{
    aiString filePath{};
    if (material->GetTexture(aiTextureType::aiTextureType_OPACITY, 0, &filePath) == AI_SUCCESS)
        m_opacity = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eOpacity));
    else
        m_opacity = BLEACH_NEW(Texture("", Material::eOpacity));
}

nest::Texture* nest::PBRTextureStack::GetTexture(Material::TextureFlags type)
{
    switch (type)
    {
    case Material::eDiffuse:
        return m_diffuse;
    case Material::eRoughness:
        return m_roughness;
    case Material::eAO:
        return m_ambientOcclusion;
    case Material::eMetallic:
        return m_metallic;
    case Material::eNormal:
        return m_normal;
    case Material::eOpacity:
        return m_opacity;
    default:
        return nullptr;
    }
}
