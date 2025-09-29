#include "ImGuiManager.h"
#include "../SDLWrappers/Window.h"
#include "../Engine/Engine.h"
#include "../VulkanWrappers/Vulkan.h"
#include "GraphicsSystem.h"
#include "WindowManager.h"	
#include "CameraManager.h"
#include "LightManager.h"
#include "../VulkanWrappers/Texture.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../VulkanWrappers/DescriptorSet.h"
#include "../VulkanWrappers/Sampler.h"
#include "../VulkanWrappers/ImageView.h"
#include "../VulkanWrappers/Image.h"
#include "../lib/gap-311/VulkanWrapper.hpp"
#include "../lib/gap-311/VulkanShaderLibrary.hpp"
#include "../VulkanWrappers/PipelineConfigs.h"
#include "../Objects/CameraObject.h"
#include "../Objects/Components/TransformComponent3D.h"
#include "../Objects/Components/KinematicComponent3D.h"
#include "../Nodes/Node.h"
#include <imgui.h>
#include <SDL.h>
#include <vulkan/vulkan.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <SDL_vulkan.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <assimp/types.h>
bool nest::ImGuiManager::Init()
{
	if (!ImGuiInitialize())
	{
		return false;
	}
    return true;
}

void nest::ImGuiManager::ImGuiNewFrame() const
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}
template <typename Iter, typename Value>
int IndexOf(Iter first, const Iter last, const Value& val, int defaultIndex = 0)
{
	auto loc = std::find(first, last, val);
	if (loc == last)
		return defaultIndex;
	return static_cast<int>(std::distance(first, loc));
}

template <typename StoredValue, typename Value>
bool CheckIfChanged(StoredValue& val, const Value& newVal)
{
	bool changed = (val != newVal);
	val = newVal;
	return changed;
}


bool CheckIfLightChanged(const nest::Light& left, const nest::Light& right)
{
	// ugly
	// I could just do this in struct file..
	if (left.attenuation == right.attenuation &&
		left.diffuse == right.diffuse &&
		left.type == right.type &&
		left.innerCone == right.innerCone &&
		left.outerCone == right.outerCone &&
		left.position == right.position &&
		left.direction == right.direction &&
		left.lightRange == right.lightRange)
		return false;
	return true;
}



void nest::ImGuiManager::PreRender() const
{
	auto map = nest::LightManager::s_samplerShadowMap;
	ImGuiNewFrame();
	auto cameraTransform = Engine::GetManager<nest::CameraManager>()->GetCamera()->GetComponent<TransformComponent3D>();
	auto& cameraSettings = Engine::GetManager<nest::CameraManager>()->GetCamera()->GetComponent<KinematicComponent3D>()->GetKins();
	auto& shadowSettings = Engine::GetGraphics()->GetShadowSettings();
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
        // Fix: Cast vk::DescriptorSet to ImTextureID (which is void*) for ImGui::Image
		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::DragFloat3("Camera Position", glm::value_ptr(cameraTransform->GetPositionRef()));
			ImGui::DragFloat("Camera Linear Speed", &cameraSettings.m_maxLinearSpeed);
			ImGui::DragFloat("Camera Rotation Speed", &cameraSettings.m_maxRotationSpeed);
		}
		if (ImGui::CollapsingHeader("Shadow Map"))
		{
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(set->set)), { 256, 256 });
			ImGui::DragFloat("Depth Bias Constant", &shadowSettings.depthBiasConstant);
			ImGui::DragFloat("Depth Bias Slope", &shadowSettings.depthBiasSlope);
			ImGui::DragFloat("Light Far", &shadowSettings.lightFar);
			ImGui::DragFloat("Light Near", &shadowSettings.lightNear);
			ImGui::DragFloat("Light FOV", &shadowSettings.lightFOV);
		}
		ImGui::SeparatorText("Pipeline");
		bool rebuildNeeded = false;

		static const char* polygonModeNames[] = { "Fill", "Line" };
		static const std::array polygonModeValues = { vk::PolygonMode::eFill, vk::PolygonMode::eLine };
		int polygonModeIndex = IndexOf(polygonModeValues.begin(), polygonModeValues.end(), s_polygonMode);
		ImGui::Combo("Polygon Mode", &polygonModeIndex, polygonModeNames, _countof(polygonModeNames));
		rebuildNeeded |= CheckIfChanged(s_polygonMode, polygonModeValues[polygonModeIndex]);

		static const char* cullModeNames[] = { "None", "Front", "Back", "Front and Back" };
		static const std::array cullModeValues = { vk::CullModeFlagBits::eNone, vk::CullModeFlagBits::eFront, vk::CullModeFlagBits::eBack, vk::CullModeFlagBits::eFrontAndBack };
		int cullModeIndex = IndexOf(cullModeValues.begin(), cullModeValues.end(), s_cullMode);
		ImGui::Combo("Cull Mode", &cullModeIndex, cullModeNames, _countof(cullModeNames));
		rebuildNeeded |= CheckIfChanged(s_cullMode, cullModeValues[cullModeIndex]);

		static const char* frontFaceNames[] = { "Clockwise", "Counter Clockwise" };
		static const std::array frontFaceValues = { vk::FrontFace::eClockwise, vk::FrontFace::eCounterClockwise };
		int frontFaceIndex = IndexOf(frontFaceValues.begin(), frontFaceValues.end(), s_frontFace);
		ImGui::Combo("Front Face", &frontFaceIndex, frontFaceNames, _countof(frontFaceNames));
		rebuildNeeded |= CheckIfChanged(s_frontFace, frontFaceValues[frontFaceIndex]);

		ImGui::Checkbox("Depth Test Enable", &s_depthTest);
		rebuildNeeded |= CheckIfChanged(s_depthTest, s_depthWrite);
		ImGui::Checkbox("Depth Write Enable", &s_depthWrite);
		rebuildNeeded |= CheckIfChanged(s_depthTest, s_depthWrite);

		static const char* depthCompareOpNames[] = { "Never", "Always", "Equal", "Not Equal", "Greater", "Greater or Equal", "Less", "Less or Equal" };
		static const std::array depthCompareOpValues = { vk::CompareOp::eNever, vk::CompareOp::eAlways, vk::CompareOp::eEqual, vk::CompareOp::eNotEqual, vk::CompareOp::eGreater, vk::CompareOp::eGreaterOrEqual, vk::CompareOp::eLess, vk::CompareOp::eLessOrEqual };
		int depthCompareOpIndex = IndexOf(depthCompareOpValues.begin(), depthCompareOpValues.end(), s_compareOp);
		ImGui::Combo("Depth Compare Op", &depthCompareOpIndex, depthCompareOpNames, _countof(depthCompareOpNames));
		rebuildNeeded |= CheckIfChanged(s_compareOp, depthCompareOpValues[depthCompareOpIndex]);

		if (rebuildNeeded)
			nest::Engine::GetGraphics()->RebuildMeshPipelines();
		int count = 0;
		auto lightManager = Engine::GetManager<LightManager>();
		Light lightData{};
		for (auto& light : lightManager->GetLights())
		{
			if (ImGui::CollapsingHeader("Light"))
			{
				lightData = light->GetData();
				rebuildNeeded = false;
				ImGui::DragFloat3("Position", glm::value_ptr(lightData.position));
				ImGui::DragFloat3("Atten", glm::value_ptr(lightData.attenuation));
				ImGui::InputFloat3("Direction", glm::value_ptr(lightData.direction));
				ImGui::InputFloat("LightRange", &lightData.lightRange, 1.0, 5.0);
				ImGui::InputFloat("InnerCone", &lightData.innerCone, 1.0, 5.0);
				ImGui::InputFloat("OuterCone", &lightData.outerCone, 1.0, 5.0);
				ImGui::InputFloat3("diffuse", glm::value_ptr(lightData.diffuse));
				const char* types[] = { "Point", "Sun", "Spot", "Area" }; // These types must match the order of the enum
				int typeIndex = static_cast<int>(lightData.type);
				if (ImGui::ListBox("Type", &typeIndex, types, IM_ARRAYSIZE(types), 4))
				{
					lightData.type = static_cast<decltype(Light::type)>(typeIndex);
				}
				rebuildNeeded |= CheckIfLightChanged(lightData, light->GetData());
				if (rebuildNeeded)
				{
					lightData.cutoff = glm::cos(glm::radians(lightData.outerCone));
					light->SetData(lightData);
				}

			}
			++count;
		}
	}
	ImGui::End();
}

void nest::ImGuiManager::Render(vk::CommandBuffer& commands)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands);
}

void nest::ImGuiManager::Destroy()
{
	//clear font textures from cpu data
	ImGui_ImplVulkan_RemoveTexture(set->set);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	auto vk = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan;
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().destroySampler(sampler->sampler);
	BLEACH_DELETE(sampler);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().destroyImageView(view->view);
	BLEACH_DELETE(view);
	BLEACH_DELETE(set);
}

void nest::ImGuiManager::HandleEvent([[maybe_unused]] const Event& evt)
{
	//
}

void nest::ImGuiManager::HandleSDLEvent(const SDL_Event& evt)
{
	ImGui_ImplSDL2_ProcessEvent(&evt);
}

bool nest::ImGuiManager::ImGuiInitialize()
{
	auto vulkan = Engine::GetGraphics()->m_vulkanWrapper->vulkan;
	auto window = Engine::GetManager<nest::WindowManager>()->GetWindow();
	auto device = vulkan.GetDevice();
	auto gpuDevice = vulkan.GetPhysicalDevice();
	auto renderPass = vulkan.GetDisplayRenderPass();
	auto graphicsQue = vulkan.GetGraphicsQueue();
	auto instance = vulkan.GetInstance();
	
	IMGUI_CHECKVERSION();
	if (!ImGui::CreateContext())
	{
		_LOG(LogType::kError, "Failed to create ImGui context!");
		return false;
	}

	ImGui::StyleColorsDark();
	assert(ImGui_ImplSDL2_InitForVulkan(window));
	

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = vulkan.GetInstance();
	initInfo.PhysicalDevice = vulkan.GetPhysicalDevice();
	initInfo.Device = vulkan.GetDevice();
	initInfo.QueueFamily = vulkan.GetGraphicsQueueIndex();
	initInfo.Queue = vulkan.GetGraphicsQueue();
	initInfo.PipelineCache = vulkan.GetPipelineCache();
	initInfo.DescriptorPool = vulkan.GetDescriptorPool();
	initInfo.RenderPass = vulkan.GetDisplayRenderPass();
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = vulkan.GetSwapchain().image_count;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;
	if (!ImGui_ImplVulkan_Init(&initInfo))
	{
		_LOG(LogType::kError, "Failed to init ImGui Vulkan backend.");
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
		return false;
	}
	set = BLEACH_NEW(DescriptorSet{});
	view = BLEACH_NEW(ImageView{});
	sampler = BLEACH_NEW(Sampler{});
	sampler->sampler = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
		.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
		.setMinFilter(vk::Filter::eNearest)
		.setMagFilter(vk::Filter::eNearest)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
	);
	view->view = vulkan.GetDevice().createImageView(
		vk::ImageViewCreateInfo()
		.setImage(nest::LightManager::s_targetShadowMap.images[0])
		.setFormat(vk::Format::eD16Unorm)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(
			vk::ImageSubresourceRange()
			.setLayerCount(1)
			.setLevelCount(1)
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
		)
		.setComponents(
			vk::ComponentMapping()
			.setR(vk::ComponentSwizzle::eIdentity)
			.setG(vk::ComponentSwizzle::eR)
			.setB(vk::ComponentSwizzle::eR)
			.setA(vk::ComponentSwizzle::eR)
		)
	);
	set->set = ImGui_ImplVulkan_AddTexture(sampler->sampler, view->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return true;
}
