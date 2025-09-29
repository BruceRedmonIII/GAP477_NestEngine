#include "GraphicsSystem.h"
#include "../Configuration/EngineConfig.h"
#include "../Engine/Engine.h"
#include "../Factories/GameFactory.h"
#include "../GlobalConstants/constants.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "WindowManager.h"
#include "ImGuiManager.h"
#include "../Nodes/Node.h"
#include "../Scene/Scene.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../VulkanWrappers/Pipeline.h"
#include "../VulkanWrappers/PipelineConfigs.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Math/MathAlias.h"
#include "SkyboxManager.h"
#include <algorithm>
#include <SDL.h>
void RenderOpaque(std::vector<nest::BaseNode*>& vec, vk::CommandBuffer& commands)
{
	for (auto it = vec.begin(); it != vec.end(); /* */)
	{
		auto* node = *it;
		if (node->IsOpaque())
		{
			node->Render(commands);
			it = vec.erase(it);
		}
		else
			++it;
	}
}

void RenderTransparent(std::vector<nest::BaseNode*>& vec, vk::CommandBuffer& commands)
{
	while (!vec.empty())
	{
		vec.back()->Render(commands);
		vec.pop_back();
	}
}

void nest::GraphicsSystem::Render()
{
	vk::CommandBuffer commands;
	vk::Framebuffer framebuffer;
	if (!m_vulkanWrapper->vulkan.BeginFrame(commands, framebuffer))
		return;

	// Update all of our buffers so we get accurate results
	nest::Engine::GetInstance()->UpdateManagerVKBuffers(commands);

	// Draw Shadows is a separate render pass, therefore we must draw them before drawing objects
	// While objects haven't been rendered yet, we can still see their vertices using our render queue
	// so we can do this before even drawing the objects
	RenderShadows(commands);

	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(m_vulkanWrapper->vulkan.GetDisplayRenderPass());
	rpbi.setFramebuffer(framebuffer);

	vk::ClearValue s_clearValues[2] = // the color to clear the area with
	{
		vk::ClearColorValue(0.05f, 0.05f, 0.05f, 0.0f),
		vk::ClearDepthStencilValue(1.0f, 0),
	};
	rpbi.setClearValues(s_clearValues);
	rpbi.setRenderArea({ { 0, 0 }, m_vulkanWrapper->vulkan.GetSwapchain().extent });

	// Begin Render Pass
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	// Render
#if _RENDER_SKYBOX
	nest::Engine::GetManager<nest::SkyboxManager>()->RenderSkybox(commands);
#endif
#if _RENDER_GRID == 1
	RenderGrid(commands);
#endif
	if (!m_renderQueue.empty())
	{
		SortQue();
		for (auto& node : m_renderQueue)
		{
			node->Render(commands);
		}
	}

	// Cleanup
	m_renderQueue.clear();
	nest::Engine::GetManager<nest::ImGuiManager>()->Render(commands);
	commands.endRenderPass();
	m_vulkanWrapper->vulkan.EndFrame();
}


//------------------------------------------------------------------------------
// Private

//------------------------------------------------------------------------------
// Init functions
bool nest::GraphicsSystem::InitVulkanWrapper()
{
#ifdef _DEBUG
	constexpr bool enableBreakOnError = true;
#else
	constexpr bool enableBreakOnError = false;
#endif

	GAP311::VulkanWrapper::ConfigOptions vulkanOptions;
	vulkanOptions.configureInstanceFunc = [](vkb::InstanceBuilder& builder) -> bool
		{
			builder.require_api_version(1, 3);
			return true;
		};
	vulkanOptions.configureDeviceFunc = [](vkb::PhysicalDeviceSelector& selector) -> bool
		{
			selector.set_required_features(
				vk::PhysicalDeviceFeatures()
				.setFillModeNonSolid(true) // support wireframe
			);
			return true;
		};
	vulkanOptions.debugLogFunc = GAP311::VulkanWrapper::VulkanLogToSDL(enableBreakOnError);
	vulkanOptions.enableDepthStencilBuffer = true;
	if (!m_vulkanWrapper->vulkan.Initialize(s_kAppName, GAP311::VulkanWrapper::CreateVulkanSurfaceSDL(nest::Engine::GetManager<nest::WindowManager>()->GetWindow()), vulkanOptions))
	{
		_LOG(LogType::kError, "Failed initializing VulkanWrapper.");
		SDL_Quit();
		return false;
	}
	return true;
}

bool nest::GraphicsSystem::InitShaders()
{
	GAP311::VulkanShaderLibrary::ConfigOptions shaderOptions;
	shaderOptions.logMessage = [](const char* msg)
		{
			_LOG_V(LogType::kLog, "ShaderLibrary: %s", msg);
		};
	shaderOptions.searchDirs = {
		std::filesystem::current_path().string(), // working dir
		std::filesystem::path(s_kShaderPath).parent_path().string(), // exe dir
		std::filesystem::path(s_kShaderPath).string(), // exe dir

		std::filesystem::relative(std::filesystem::path(__FILE__).parent_path()).string(), // source file dir
	};
	if (!m_vulkanWrapper->shaderLib.Initialize(m_vulkanWrapper->vulkan.GetDevice(), shaderOptions))
	{
		_LOG(LogType::kError, "Failed initializing VulkanShaderLibrary.");
		m_vulkanWrapper->vulkan.Shutdown();
		SDL_Quit();
		return false;
	}
	return true;
}

bool nest::GraphicsSystem::Init()
{
	m_shadowSettings.lightFOV = glm::radians(m_shadowSettings.lightFOV);
	m_vulkanWrapper = BLEACH_NEW(Vulkan());
	if (!InitVulkanWrapper())
		return false;
	if (!InitShaders())
		return false;
	return true;
}

bool nest::GraphicsSystem::InitPipelines([[maybe_unused]] nest::Scene* scene)
{
	m_pPipelines.emplace_back(CreateMeshPipeline());
	m_pPipelines.emplace_back(CreateGridPipeline());
	m_pPipelines.emplace_back(CreateTexturedMeshPipeline());
	m_pPipelines.emplace_back(CreateShadowMapPipeline());
	m_pPipelines.emplace_back(CreateSkyboxPipeline());
	return true;
}

void nest::GraphicsSystem::RebuildMeshPipelines()
{
	for (auto& pipeline : m_pPipelines)
	{
		if (pipeline->id == s_kMeshPipeline)
			RebuildMeshPipeline(pipeline);
		else if (pipeline->id == s_kTexturedMeshPipeline)
			RebuildTexturedMeshPipeline(pipeline);
	}
}

// Currently RenderShadows only works for directional lights 
// To modify this to work for point lights, we have to adjust the center for
// our view matrix. To do this we also need to allocate 6 shadow maps to store the different directions
// of the point light
void nest::GraphicsSystem::RenderShadows(vk::CommandBuffer& commands)
{
	constexpr vk::ClearValue clearValues[] =
	{
		vk::ClearDepthStencilValue{ 1.0f, 0 },
	};
	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(nest::LightManager::s_targetShadowMap.renderPass);
	rpbi.setFramebuffer(nest::LightManager::s_targetShadowMap.framebuffer);
	rpbi.setClearValues(clearValues);
	rpbi.setRenderArea({ { 0, 0 }, nest::LightManager::s_lightMapDimensions });
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	commands.setDepthBias(m_shadowSettings.depthBiasConstant, 0.0f, m_shadowSettings.depthBiasSlope);

	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, GetPipeline(s_kShadowMapPipeline)->pipeline->GetPipeline());
	CameraManager* camManager = nest::Engine::GetManager<CameraManager>();
	auto uniform = camManager->GetActiveCameraUniform();
	//auto lightManager = nest::Engine::GetManager<LightManager>();
	ShadowPushConstants pushConstants;
	//for (int i = 0; i < lightManager->m_uniform.lightCount; ++i)
	//{
		Vec3 eyePos = uniform.light0.position;
		// sun lights point to the origin
		Vec3 center = { 0.0f, 0.0f, 0.0f };
		Vec3 up = { 0.0f, 1.0f, 0.0f };
		// Check if eye position is too close to center to avoid zero-length forward vector
		if (glm::length(eyePos - center) < 0.001f)
		{
			// Move eyePos slightly away from center to avoid NaNs
			eyePos = { 0.0f, 0.0f, 1.0f };
		}
		Mat4 lightViewMatrix = glm::lookAt(eyePos, center, up);
		Mat4 lightProjMatrix{};
		if (uniform.light0.type == Light::eSpot)
		{
			lightProjMatrix = glm::ortho(-uniform.light0.outerCone, uniform.light0.outerCone, -uniform.light0.outerCone, uniform.light0.outerCone, m_shadowSettings.lightNear, uniform.light0.lightRange);
		}
		else
		{
			lightProjMatrix = glm::perspective(m_shadowSettings.lightFOV, m_shadowSettings.lightAspect, m_shadowSettings.lightNear, m_shadowSettings.lightFar);
		}
		Mat4 lightProjView = lightProjMatrix * lightViewMatrix;
		uniform.light0.viewProj = lightProjView;
		camManager->SetCameraUniform(uniform);
		pushConstants.light0Matrix = lightProjView;


#if _RENDER_SHADOWS == 1
		for (auto& mesh : m_renderQueue)
		{
			if (mesh->GetTransformData().material.options & Material::OptionFlags::eCastShadows)
			{
				pushConstants.objectMatrix = mesh->GetTransform();
				commands.pushConstants(GetPipeline(s_kShadowMapPipeline)->pipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
				mesh->Draw(commands);
			}
		}
#endif
	//}
	commands.endRenderPass();
}

// Init functions
//------------------------------------------------------------------------------
void nest::GraphicsSystem::RenderGrid(vk::CommandBuffer& commands)
{
	PushConstants constants;
	constants.objectMatrix = glm::identity<glm::mat4>();
	auto* grid = GetPipeline(s_kGridPipeline);
	commands.pushConstants(grid->pipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(constants), &constants);
	commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, grid->pipeline->GetLayout(), 0, { nest::CameraManager::s_cameraUniform }, { });
	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, grid->pipeline->GetPipeline());
	commands.draw(6, 1, 0, 0);
}

bool cmp(nest::MeshNode* lhs, nest::MeshNode* rhs)
{
	nest::CameraManager* camManager = nest::Engine::GetManager<nest::CameraManager>();
	auto uniform = camManager->GetActiveCameraUniform();
	auto d1 = glm::distance(uniform.cameraPosition, lhs->GetPos());
	auto d2 = glm::distance(uniform.cameraPosition, rhs->GetPos());
	if (lhs->IsOpaque() && rhs->IsOpaque())
		return d1 < d2;
	 if (lhs->IsOpaque() && !rhs->IsOpaque())
		return true;
	 if (!lhs->IsOpaque() && rhs->IsOpaque())
		return false;
	return d1 > d2;
}

void nest::GraphicsSystem::SortQue()
{
	std::ranges::sort(m_renderQueue.begin(), m_renderQueue.end(), cmp);
}

void nest::GraphicsSystem::QueRender(MeshNode* node)
{
	m_renderQueue.emplace_back(node);
}

// Private
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Getter
const nest::Pipeline* nest::GraphicsSystem::GetPipeline(HashedId id)
{
	for (auto& pipeline : m_pPipelines)
	{
		if (pipeline->id == id)
			return pipeline;
	}
	_LOG(LogType::kError, "No pipeline found with matching id!");
	return m_pPipelines[0];
}

// Getter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Cleanup
void nest::GraphicsSystem::Exit()
{
	m_vulkanWrapper->vulkan.GetDevice().waitIdle();
	for (auto& pipeline : m_pPipelines)
	{
		m_vulkanWrapper->vulkan.DestroyPipeline(pipeline->pipeline);
		BLEACH_DELETE(pipeline);
	}
	// Shaderlib must be shutdown before vulkan as its dependent of vulkan
	m_vulkanWrapper->shaderLib.Shutdown();
	m_vulkanWrapper->vulkan.Shutdown();
	BLEACH_DELETE(m_vulkanWrapper);
}
// Cleanup
//-----------------------------------------------------------------------------