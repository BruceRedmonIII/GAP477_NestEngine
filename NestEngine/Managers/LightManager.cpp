#include "LightManager.h"
#include "GraphicsSystem.h"
#include "../Configuration/LogMacro.hint"
#include "../VulkanWrappers/Vulkan.h"
#include "../Scene/Scene.h"
#include "../Engine/Engine.h"
#include "../Nodes/Node.h"
#include "CameraManager.h"
vk::Extent2D nest::LightManager::s_lightMapDimensions = { 2048 , 2048 };
vk::Sampler nest::LightManager::s_samplerShadowMap{};

GAP311::ShaderUniform nest::LightManager::s_lightUniform{};
GAP311::ShaderUniform nest::LightManager::s_shadowMapUniform{};
vk::Buffer nest::LightManager::s_lightConstantsBuffer{};

GAP311::RenderTarget nest::LightManager::s_targetShadowMap{};
bool nest::LightManager::Init()
{
	// this is creating an image and an image view
	// we us the image to create our shadow map, we pass this into the shadow pipeline
	// and it will color the pixels that will be casting shadows
	s_samplerShadowMap = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
		.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
		.setMinFilter(vk::Filter::eNearest)
		.setMagFilter(vk::Filter::eNearest)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
		.setCompareEnable(true)
		.setCompareOp(vk::CompareOp::eLessOrEqual)
	);
	s_targetShadowMap = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateRenderTarget(s_lightMapDimensions, vk::Format::eUndefined, vk::Format::eD16Unorm);
	s_lightConstantsBuffer = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformBuffer(sizeof(LightConstants));
	s_lightUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForBuffer(LightConstants::binding, s_lightConstantsBuffer, sizeof(LightConstants));
	s_shadowMapUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(2, s_targetShadowMap.views[0], s_samplerShadowMap);

	m_maxLightCount = m_uniform.lightCount;

	m_uniform.lightCount = 0;
	m_currentLightCount = 0;

    return true;
}

void nest::LightManager::Exit()
{
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_lightUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyBuffer(s_lightConstantsBuffer);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().destroySampler(s_samplerShadowMap);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyRenderTarget(s_targetShadowMap);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_shadowMapUniform);
}

void nest::LightManager::LoadLightsFromScene(const Scene* scene)
{
	for (LightNode* node : scene->GetLights())
	{
		AddLight(node);
	}
}

bool nest::LightManager::AddLight(LightNode* light)
{
	if (m_currentLightCount < m_maxLightCount)
	{
		m_uniform.lights[m_currentLightCount] = light->GetData();
		m_lightNodes.emplace_back(light);
		++m_currentLightCount;
		++m_uniform.lightCount;
		return true;
	}
	return false;
}

void nest::LightManager::UpdateVKBuffer(vk::CommandBuffer& commands)
{
	auto uniform = nest::Engine::GetManager<CameraManager>()->GetActiveCameraUniform();
	for (int i = 0; i < m_uniform.lightCount; ++i)
	{
		m_uniform.lights[i] = m_lightNodes[i]->GetData();
	}
	uniform.light0 = m_uniform.lights[0];
	commands.updateBuffer(s_lightConstantsBuffer, 0, sizeof(LightConstants), &m_uniform);
	nest::Engine::GetManager<CameraManager>()->SetCameraUniform(uniform);
}