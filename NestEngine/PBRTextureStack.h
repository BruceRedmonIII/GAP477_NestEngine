#pragma once
#include "VulkanWrappers/Texture.h"
#include "Uniforms/MeshDataStructs.h"
namespace nest
{
    class PBRTextureStack
    {
        Texture* m_diffuse = nullptr;
        Texture* m_normal = nullptr;
        Texture* m_roughness = nullptr;
        Texture* m_metallic = nullptr;
        Texture* m_ambientOcclusion = nullptr;
        Texture* m_opacity = nullptr;
        // If the texture stack contains at-least a diffuse texture
        // then we can use this
        bool m_isValid = false;
    public:
        PBRTextureStack(const std::string& path, aiMaterial* material);
        ~PBRTextureStack();
        bool IsValid() const { return m_isValid; };
        Texture* GetTexture(Material::TextureFlags type);
    private:
        void ResolveDiffuse(aiMaterial* material);
        void ResolveNormal(aiMaterial* material);
        void ResolveRoughness(aiMaterial* material);
        void ResolveMetallic(aiMaterial* material);
        void ResolveAO(aiMaterial* material);
        void ResolveOpacity(aiMaterial* material);
    };
}