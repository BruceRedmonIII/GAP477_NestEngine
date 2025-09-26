#pragma once
#include "Manager.h"
#include <vector>
namespace nest
{
    struct DescriptorSet;
    struct Image;
    struct ImageView;
    struct Sampler;
    class MeshNode;
    class SkyboxManager : public Manager
    {
        MeshNode* m_cubeMesh = nullptr;
        const char* m_images[6];
        std::vector<uint8_t> m_imageBytes;
        Image* textureCubeImage;
        ImageView* textureCubeImageView;
        Sampler* textureCubeSampler;
    public:
        static GAP311::ShaderUniform s_skyboxUniform;
    public:
        bool Init(const char* images[6]);
        void Exit() override;
        void RenderSkybox(vk::CommandBuffer& commands);
    };
}