#include "SkyboxManager.h"
#include "../VulkanWrappers/Texture.h"
#include "../VulkanWrappers/Image.h"
#include "../VulkanWrappers/ImageView.h"
#include "../VulkanWrappers/Sampler.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Engine/Engine.h"
#include "../Managers/GraphicsSystem.h"
#include "../VulkanWrappers/Pipeline.h"
#include "../Factories/ShapeFactory.h"
#include "../GlobalConstants/constants.h"
#include "../Nodes/Node.h"
#include "CameraManager.h"
#include <ranges>
#include <SDL.h>
#include "FileManager.h"

GAP311::ShaderUniform nest::SkyboxManager::s_skyboxUniform{};

bool nest::SkyboxManager::Init(const char* images[6])
{
	textureCubeImage = BLEACH_NEW(Image);
	textureCubeImageView = BLEACH_NEW(ImageView);
	textureCubeSampler = BLEACH_NEW(Sampler);
	ShapeFactory factory;
	m_cubeMesh = BLEACH_NEW(MeshNode);
	factory.LoadCube(m_cubeMesh);
	for (int i = 0; i < 6; ++i)
	{
		SDL_Surface* side = nest::Texture::LoadCompatibleImage(nest::FileManager::ResolvePath(s_kSkyboxKey, images[i]));
		if (!side) // LoadCompatibleImage sends the error message, so we just return false
			return false;

		const uint8_t* start = static_cast<const uint8_t*>(side->pixels);
		const uint8_t* end = &static_cast<const uint8_t*>(side->pixels)[side->w * side->h * side->format->BytesPerPixel];

		m_imageBytes.insert(m_imageBytes.end(), start, end);
	}
	textureCubeSampler->sampler = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setMinFilter(vk::Filter::eLinear)
		.setMagFilter(vk::Filter::eLinear)
	);

	std::tie(textureCubeImage->image, textureCubeImageView->view) = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateCubemapImageAndView(
		{ 1024, 1024 },
		GAP311::VulkanWrapper::VulkanFormatFromSDLFormat(SDL_PIXELFORMAT_ABGR8888),
		m_imageBytes.data(), m_imageBytes.size()
	);
	s_skyboxUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(3, textureCubeImageView->view, textureCubeSampler->sampler);
    return true;
}

void nest::SkyboxManager::Exit()
{
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_skyboxUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().destroySampler(textureCubeSampler->sampler);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyImageAndView(textureCubeImage->image, textureCubeImageView->view);
	BLEACH_DELETE(textureCubeImage);
	BLEACH_DELETE(textureCubeImageView);
	BLEACH_DELETE(textureCubeSampler);
	BLEACH_DELETE(m_cubeMesh);
}

void nest::SkyboxManager::RenderSkybox(vk::CommandBuffer& commands)
{
	commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, nest::Engine::GetGraphics()->GetPipeline(s_kSkyboxPipeline)->pipeline->GetLayout(), 0, { nest::CameraManager::s_cameraUniform, s_skyboxUniform}, {});
	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, nest::Engine::GetGraphics()->GetPipeline(s_kSkyboxPipeline)->pipeline->GetPipeline());
	commands.bindIndexBuffer(m_cubeMesh->m_indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
	commands.bindVertexBuffers(0, { m_cubeMesh->m_vertexBuffer->GetBuffer() }, { 0 });
	commands.drawIndexed(m_cubeMesh->m_indexCount, 1, 0, 0, 0);
}
