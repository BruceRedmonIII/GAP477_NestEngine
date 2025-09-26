#if _DEMO
#include <gap-311/VulkanWrapper.hpp>
#include <gap-311/VulkanShaderLibrary.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>

#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace
{
	bool ImGuiInitialize(SDL_Window*, GAP311::VulkanWrapper&);
	void ImGuiShutdown();
	void ImGuiNewFrame();
	void ImGuiRender(vk::CommandBuffer& commands);
	void ImGuiOnEvent(const SDL_Event& event);
}

using namespace GAP311;

extern struct Camera* CameraCreate();
extern void CameraDestroy(Camera* pCamera);
extern void CameraGetViewMatrix(Camera* pCamera, glm::mat4* pMatrix);
extern void CameraUpdate(Camera* pCamera, float tickDelta);
extern void CameraOnEvent(Camera* pCamera, const SDL_Event& event);

struct MeshVertex
{
	glm::vec3 position;
};

struct PushConstants
{
	glm::mat4 objectMatrix = glm::identity<glm::mat4>();
	glm::vec4 objectColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct CameraConstants
{
	static constexpr uint32_t binding = 0;

	glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
	glm::mat4 viewMatrix = glm::identity<glm::mat4>();
};

VulkanWrapper vulkan;
VulkanShaderLibrary shaderLib;
std::shared_ptr<VulkanWrapper::GraphicsPipeline> gridPipeline;
std::shared_ptr<VulkanWrapper::GraphicsPipeline> meshPipeline;
vk::Buffer cubeIndexBuffer;
vk::Buffer cubeVertexBuffer;
uint32_t cubeIndexCount = 0;
std::vector<glm::mat4> cubeTransforms;
glm::vec4 cubeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
vk::Buffer pyramidIndexBuffer;
vk::Buffer pyramidVertexBuffer;
uint32_t pyramidIndexCount = 0;
std::vector<glm::mat4> pyramidTransforms;

struct Camera* camera = nullptr;
CameraConstants cameraConstants;
VulkanWrapper::ShaderUniform cameraUniform;
vk::Buffer cameraConstantsBuffer;

vk::ClearValue clearValues[] =
{
	vk::ClearColorValue(0.05f, 0.05f, 0.05f, 0.0f),
	vk::ClearDepthStencilValue( 1.0f, 0 ),
};

vk::PolygonMode meshPolygonMode = vk::PolygonMode::eFill;
vk::CullModeFlags meshCullMode = vk::CullModeFlagBits::eBack;
vk::FrontFace meshFrontFace = vk::FrontFace::eCounterClockwise;
bool meshDepthTestEnable = true;
vk::CompareOp meshDepthCompareOp = vk::CompareOp::eLess;
bool meshDepthWriteEnable = true;

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

void RebuildMeshPipeline()
{
	vulkan.GetDevice().waitIdle();

	if (meshPipeline)
		vulkan.DestroyPipeline(meshPipeline);

	meshPipeline = vulkan.CreatePipeline(
	{
		.vertexBindings =
		{
			vk::VertexInputBindingDescription()
				.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(MeshVertex)),
		},
		.vertexAttributes =
		{
			vk::VertexInputAttributeDescription()
				.setLocation(0)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::position)),
		},
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setOffset(0)
				.setSize(sizeof(PushConstants))
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
		},
		.shaderUniforms =
		{
			cameraUniform,
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName("main")
				.setModule(shaderLib.GetModule("general.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName("main")
				.setModule(shaderLib.GetModule("general.frag"))
		},
		.polygonMode = meshPolygonMode,
		.cullMode = meshCullMode,
		.frontFace = meshFrontFace,
		.depthTestEnable = meshDepthTestEnable,
		.depthCompareOp = meshDepthCompareOp,
		.depthWriteEnable = meshDepthWriteEnable,
		/*.colorBlendStates =
		{
			vk::PipelineColorBlendAttachmentState()
				.setBlendEnable(true)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setAlphaBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
		},*/
	});
}

std::tuple<vk::Buffer, vk::Buffer, uint32_t> LoadCubeAsset()
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile("demos/05-asset-loading/cube.glb", aiProcess_ConvertToLeftHanded);
	if (!scene)
	{
		SDL_Log("Failed to load cube!");
		return {};
	}

	SDL_Log("Meshes: %d", scene->mNumMeshes);

	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;

	const auto& mesh = scene->mMeshes[0];

	if (mesh->mMaterialIndex < scene->mNumMaterials)
	{
		const auto& material = scene->mMaterials[mesh->mMaterialIndex];

		for (uint32_t p = 0; p < material->mNumProperties; ++p)
		{
			SDL_Log("Property: %s", material->mProperties[p]->mKey.C_Str());
		}

		aiColor4D rgba;
		material->Get(AI_MATKEY_BASE_COLOR, rgba);
		cubeColor = { rgba.r, rgba.g, rgba.b, rgba.a };
	}

	for (uint32_t v = 0; v < mesh->mNumVertices; ++v)
	{
		const auto& vert = mesh->mVertices[v];

		vertices.push_back({ { vert.x, vert.y, vert.z } });
	}

	for (uint32_t f = 0; f < mesh->mNumFaces; ++f)
	{
		const auto& face = mesh->mFaces[f];

		for (uint32_t i = 0; i < face.mNumIndices; ++i)
		{
			indices.push_back(face.mIndices[i]);
		}
	}

	auto indexBuffer = vulkan.CreateIndexBuffer(indices.size() * sizeof(indices[0]), indices.data());
	auto vertexBuffer = vulkan.CreateVertexBuffer(vertices.size() * sizeof(vertices[0]), vertices.data());
	return { vertexBuffer, indexBuffer, static_cast<uint32_t>(indices.size()) };
}

bool AppStartup(SDL_Window* pWindow, const char* exePath)
{
	VulkanWrapper::ConfigOptions options{};
	options.configureInstanceFunc = [](vkb::InstanceBuilder& builder) -> bool
	{
		builder.require_api_version(1, 3);
		return true;
	};
	options.configureDeviceFunc = [](vkb::PhysicalDeviceSelector& selector) -> bool
	{
		selector.set_required_features(
			vk::PhysicalDeviceFeatures()
				.setFillModeNonSolid(true) // support wireframe
		);
		return true;
	};
	options.debugLogFunc = VulkanWrapper::VulkanLogToSDL();
	options.enableDepthStencilBuffer = true;
	if (!vulkan.Initialize(exePath, VulkanWrapper::CreateVulkanSurfaceSDL(pWindow), options))
	{
		SDL_Log("Failed initializing VulkanWrapper.");
		return false;
	}

	VulkanShaderLibrary::ConfigOptions shaderOptions{};
	shaderOptions.logMessage = [](const char* msg)
		{
			SDL_Log("ShaderLibrary: %s", msg);
		};
	shaderOptions.searchDirs = {
		std::filesystem::current_path().string(), // working dir
		std::filesystem::path(exePath).parent_path().string(), // exe dir
		std::filesystem::relative(std::filesystem::path(__FILE__).parent_path()).string(), // source file dir
	};
	if (!shaderLib.Initialize(vulkan.GetDevice(), shaderOptions))
	{
		SDL_Log("Failed initializing VulkanShaderLibrary.");
		vulkan.Shutdown();
		return false;
	}

	if (!ImGuiInitialize(pWindow, vulkan))
	{
		SDL_Log("Failed initializing ImGui.");
		shaderLib.Shutdown();
		vulkan.Shutdown();
		return false;
	}

	int windowWidth = 0, windowHeight = 0;
	SDL_GetWindowSize(pWindow, &windowWidth, &windowHeight);
	cameraConstants.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)windowWidth / windowHeight, 0.1f, 512.0f);
	cameraConstants.projectionMatrix[1][1] *= -1.0f;

	cameraConstantsBuffer = vulkan.CreateUniformBuffer(sizeof(cameraConstants));
	cameraUniform = vulkan.CreateUniformForBuffer(CameraConstants::binding, cameraConstantsBuffer, sizeof(cameraConstants));

	camera = CameraCreate();

	gridPipeline = vulkan.CreatePipeline(
	{
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setOffset(0)
				.setSize(sizeof(PushConstants))
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
		},
		.shaderUniforms =
		{
			cameraUniform,
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName("main")
				.setModule(shaderLib.GetModule("grid.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName("main")
				.setModule(shaderLib.GetModule("grid.frag"))
		},
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthTestEnable = true,
		.depthCompareOp = vk::CompareOp::eLess,
		.depthWriteEnable = false,
		.colorBlendStates = {
			vk::PipelineColorBlendAttachmentState()
				.setBlendEnable(true)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setAlphaBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
		},
	});

	RebuildMeshPipeline();

	std::tie(cubeVertexBuffer, cubeIndexBuffer, cubeIndexCount) = LoadCubeAsset();

	for (int i = 0; i < 100; ++i)
	{
		const auto scale = glm::vec3(1, 1, 1) * glm::linearRand(0.0f, 10.0f);
		const auto position = glm::ballRand(10.0f);
		cubeTransforms.push_back(glm::translate(glm::scale(glm::identity<glm::mat4>(), scale), position));
	}

	//
	const MeshVertex pyramidVertices[] =
	{
		{ {  0.0f,  0.5f,  0.0f } },
		{ { -0.5f, -0.5f, -0.5f } },
		{ { -0.5f, -0.5f,  0.5f } },
		{ {  0.5f, -0.5f,  0.5f } },
		{ {  0.5f, -0.5f, -0.5f } },
	};
	const uint32_t pyramidIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 1,
		1, 3, 2,
		3, 1, 4,
	};
	pyramidIndexCount = _countof(pyramidIndices);
	pyramidIndexBuffer = vulkan.CreateIndexBuffer(sizeof(pyramidIndices), pyramidIndices);
	pyramidVertexBuffer = vulkan.CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer, sizeof(pyramidVertices), pyramidVertices);

	for (int i = 0; i < 100; ++i)
	{
		const auto scale = glm::vec3(1, 1, 1) * glm::linearRand(0.0f, 10.0f);
		const auto position = glm::ballRand(10.0f);
		pyramidTransforms.push_back(glm::translate(glm::scale(glm::identity<glm::mat4>(), scale), position));
	}

	return true;
}

void AppShutdown()
{
	vulkan.GetDevice().waitIdle();

	CameraDestroy(camera);
	vulkan.DestroyUniform(cameraUniform);
	vulkan.DestroyBuffer(cameraConstantsBuffer);

	vulkan.DestroyPipeline(gridPipeline);

	vulkan.DestroyPipeline(meshPipeline);
	vulkan.DestroyBuffer(cubeIndexBuffer);
	vulkan.DestroyBuffer(cubeVertexBuffer);
	vulkan.DestroyBuffer(pyramidIndexBuffer);
	vulkan.DestroyBuffer(pyramidVertexBuffer);

	ImGuiShutdown();
	shaderLib.Shutdown();
	vulkan.Shutdown();
}

void AppOnTick(float tickDelta)
{
	ImGuiNewFrame();

	CameraUpdate(camera, tickDelta);
	CameraGetViewMatrix(camera, &cameraConstants.viewMatrix);

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static glm::vec2 scaleRange = { 0.1f, 10.0f };
		static float positionRadius = 10.0f;
		ImGui::DragFloat2("Random Scale Range", glm::value_ptr(scaleRange), 0.1f, 0.1f);
		ImGui::DragFloat("Random Position Radius", &positionRadius);

		if (ImGui::Button("Randomize Cubes"))
		{
			for (auto& cubeTransform : cubeTransforms)
			{
				const auto scale = glm::vec3(1, 1, 1) * glm::linearRand(scaleRange.x, scaleRange.y);
				const auto position = glm::ballRand(positionRadius);
				cubeTransform = glm::scale(glm::translate(glm::identity<glm::mat4>(), position), scale);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Randomize Pyramids"))
		{
			for (auto& pyramidTransform : pyramidTransforms)
			{
				const auto scale = glm::vec3(1, 1, 1) * glm::linearRand(scaleRange.x, scaleRange.y);
				const auto position = glm::ballRand(positionRadius);
				pyramidTransform = glm::scale(glm::translate(glm::identity<glm::mat4>(), position), scale);
			}
		}

		ImGui::SeparatorText("Clear Values");
		ImGui::ColorEdit4("Color", clearValues[0].color.float32.data());
		ImGui::DragFloat("Depth", &clearValues[1].depthStencil.depth, 0.01f, 0, 1);

		ImGui::SeparatorText("Mesh Pipeline");

		bool rebuildNeeded = false;

		static const char* polygonModeNames[] = { "Fill", "Line" };
		static const std::array polygonModeValues = { vk::PolygonMode::eFill, vk::PolygonMode::eLine };
		int polygonModeIndex = IndexOf(polygonModeValues.begin(), polygonModeValues.end(), meshPolygonMode);
		ImGui::Combo("Polygon Mode", &polygonModeIndex, polygonModeNames, _countof(polygonModeNames));
		rebuildNeeded |= CheckIfChanged(meshPolygonMode, polygonModeValues[polygonModeIndex]);

		static const char* cullModeNames[] = { "None", "Front", "Back", "Front and Back" };
		static const std::array cullModeValues = { vk::CullModeFlagBits::eNone, vk::CullModeFlagBits::eFront, vk::CullModeFlagBits::eBack, vk::CullModeFlagBits::eFrontAndBack };
		int cullModeIndex = IndexOf(cullModeValues.begin(), cullModeValues.end(), meshCullMode);
		ImGui::Combo("Cull Mode", &cullModeIndex, cullModeNames, _countof(cullModeNames));
		rebuildNeeded |= CheckIfChanged(meshCullMode, cullModeValues[cullModeIndex]);

		static const char* frontFaceNames[] = { "Clockwise", "Counter Clockwise" };
		static const std::array frontFaceValues = { vk::FrontFace::eClockwise, vk::FrontFace::eCounterClockwise };
		int frontFaceIndex = IndexOf(frontFaceValues.begin(), frontFaceValues.end(), meshFrontFace);
		ImGui::Combo("Front Face", &frontFaceIndex, frontFaceNames, _countof(frontFaceNames));
		rebuildNeeded |= CheckIfChanged(meshFrontFace, frontFaceValues[frontFaceIndex]);

		bool depthTestEnable = meshDepthTestEnable;
		ImGui::Checkbox("Depth Test Enable", &depthTestEnable);
		rebuildNeeded |= CheckIfChanged(meshDepthTestEnable, depthTestEnable);
		bool depthWriteEnable = meshDepthWriteEnable;
		ImGui::Checkbox("Depth Write Enable", &depthWriteEnable);
		rebuildNeeded |= CheckIfChanged(meshDepthWriteEnable, depthWriteEnable);
		
		static const char* depthCompareOpNames[] = { "Never", "Always", "Equal", "Not Equal", "Greater", "Greater or Equal", "Less", "Less or Equal" };
		static const std::array depthCompareOpValues = { vk::CompareOp::eNever, vk::CompareOp::eAlways, vk::CompareOp::eEqual, vk::CompareOp::eNotEqual, vk::CompareOp::eGreater, vk::CompareOp::eGreaterOrEqual, vk::CompareOp::eLess, vk::CompareOp::eLessOrEqual };
		int depthCompareOpIndex = IndexOf(depthCompareOpValues.begin(), depthCompareOpValues.end(), meshDepthCompareOp);
		ImGui::Combo("Depth Compare Op", &depthCompareOpIndex, depthCompareOpNames, _countof(depthCompareOpNames));
		rebuildNeeded |= CheckIfChanged(meshDepthCompareOp, depthCompareOpValues[depthCompareOpIndex]);

		if (rebuildNeeded)
			RebuildMeshPipeline();

		ImGui::SeparatorText("ImGui");
		static bool demoWindow = false;
		ImGui::Checkbox("Show Demo Window", &demoWindow);
		if (demoWindow)
			ImGui::ShowDemoWindow(&demoWindow);

	}
	ImGui::End();

	vk::CommandBuffer commands;
	vk::Framebuffer framebuffer;
	if (!vulkan.BeginFrame(commands, framebuffer))
		return;

	// UPDATE!
	commands.updateBuffer(cameraConstantsBuffer, 0, sizeof(cameraConstants), &cameraConstants);

	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(vulkan.GetDisplayRenderPass());
	rpbi.setFramebuffer(framebuffer);
	rpbi.setClearValues(clearValues);
	rpbi.setRenderArea({ { 0, 0 }, vulkan.GetSwapchain().extent });
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	// Draw Cubes
	for (const auto& transform : cubeTransforms)
	{
		PushConstants pushConstants;
		pushConstants.objectMatrix = transform;
		pushConstants.objectColor = cubeColor;
		commands.pushConstants(meshPipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshPipeline->GetLayout(), 0, { cameraUniform }, { });
		commands.bindPipeline(vk::PipelineBindPoint::eGraphics, meshPipeline->GetPipeline());
		commands.bindIndexBuffer(cubeIndexBuffer, 0, vk::IndexType::eUint32);
		commands.bindVertexBuffers(0, { cubeVertexBuffer }, { 0 });
		commands.drawIndexed(cubeIndexCount, 1, 0, 0, 0);
	}

	// Draw Pyramids
	for (const auto& transform : pyramidTransforms)
	{
		PushConstants pushConstants;
		pushConstants.objectMatrix = transform;
		commands.pushConstants(meshPipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshPipeline->GetLayout(), 0, { cameraUniform }, { });
		commands.bindPipeline(vk::PipelineBindPoint::eGraphics, meshPipeline->GetPipeline());
		commands.bindIndexBuffer(pyramidIndexBuffer, 0, vk::IndexType::eUint32);
		commands.bindVertexBuffers(0, { pyramidVertexBuffer }, { 0 });
		commands.drawIndexed(pyramidIndexCount, 1, 0, 0, 0);
	}

	// Draw Grid
	commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gridPipeline->GetLayout(), 0, { cameraUniform }, { });
	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, gridPipeline->GetPipeline());
	commands.draw(6, 1, 0, 0);

	ImGuiRender(commands);

	commands.endRenderPass();
	vulkan.EndFrame();
}

void AppOnEvent(const SDL_Event& event)
{
	ImGuiOnEvent(event);
	CameraOnEvent(camera, event);
}

namespace
{
	bool ImGuiInitialize(SDL_Window* pWindow, VulkanWrapper& vulkan)
	{
		IMGUI_CHECKVERSION();
		if (!ImGui::CreateContext())
		{
			SDL_Log("Failed to create ImGui context!");
			return false;
		}

		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForVulkan(pWindow);
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
			SDL_Log("Failed to init ImGui Vulkan backend.");
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		return true;
	}

	void ImGuiShutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiNewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRender(vk::CommandBuffer& commands)
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands);
	}

	void ImGuiOnEvent(const SDL_Event& event)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
	}
}
#endif