#include "PBRTextureStack.h"
#include <assimp/material.h>
#include "Managers/FileManager.h"
#include "GlobalConstants/constants.h"
#include "lib/BleachNew/BleachNew.h"
nest::PBRTextureStack::PBRTextureStack(const std::string& path, aiMaterial* material)
{
    aiString filePath{};
    material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &filePath);
    m_diffuse = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eDiffuse));
    if (m_diffuse->isValid)
        m_isValid = true;
    material->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &filePath);
    m_roughness = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eRoughness));
    material->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &filePath);
    m_ambientOcclusion = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eAO));
    material->GetTexture(aiTextureType::aiTextureType_METALNESS, 0, &filePath);
    m_metallic = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eMetallic));
    material->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &filePath);
    m_normal = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eNormal));
    material->GetTexture(aiTextureType::aiTextureType_OPACITY, 0, &filePath);
    m_opacity = BLEACH_NEW(Texture(nest::FileManager::ResolvePath(s_kAssetKey, filePath.C_Str()), Material::eOpacity));
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
