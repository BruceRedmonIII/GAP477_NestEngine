#if _DEMO
#include <gap-311/VulkanWrapper.hpp>
#include <gap-311/VulkanShaderLibrary.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
#include "backends/imgui_impl_vulkan.h"

#include <private/Transform.hpp>
#include <private/Camera.hpp>
#include <private/Utils.hpp>
#include <private/Shapes.hpp>
#include <private/DebugGrid.hpp>

#include "../../thirdparty/imgui-1.91.0/imgui.h"

using namespace GAP311;

extern bool ImGuiInitialize(SDL_Window*, VulkanWrapper&);
extern void ImGuiShutdown();
extern void ImGuiNewFrame();
extern void ImGuiRender(vk::CommandBuffer& commands);
extern void ImGuiOnEvent(const SDL_Event& event);

using GraphicsPipelinePtr = std::shared_ptr<VulkanWrapper::GraphicsPipeline>;

VulkanWrapper vulkan;
VulkanShaderLibrary shaderLib;
Private::PerspectiveCamera camera(45.0f, 1.0f);
Private::FPSFlyController cameraController(camera);
Private::DebugGrid debugGrid;

std::vector<std::filesystem::path> assetSearchDirs;

decltype(auto) ResolveAsset(const std::filesystem::path& filePath)
{
	return Private::ResolveFile(filePath, assetSearchDirs);
}

std::vector<vk::PipelineShaderStageCreateInfo> LoadShaders(const std::filesystem::path& vertex, const std::filesystem::path& fragment)
{
	return
	{
		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(shaderLib.GetModule(vertex.string().c_str()))
			.setPName("main"),
		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(shaderLib.GetModule(fragment.string().c_str()))
			.setPName("main"),
	};
}

struct DrawableMesh
{
	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;
	uint32_t elementCount;

	static DrawableMesh Create(const MeshData& mesh, VulkanWrapper& vulkan)
	{
		DrawableMesh drawable;
		drawable.vertexBuffer = vulkan.CreateVertexBuffer(mesh.vertices.size() * sizeof(mesh.vertices[0]), mesh.vertices.data());
		if (mesh.indices.empty())
		{
			drawable.elementCount = mesh.vertices.size();
		}
		else
		{
			drawable.indexBuffer = vulkan.CreateIndexBuffer(mesh.indices.size() * sizeof(mesh.indices[0]), mesh.indices.data());
			drawable.elementCount = mesh.indices.size();
		}
		return drawable;
	}
	void Cleanup(VulkanWrapper& vulkan)
	{
		vulkan.DestroyBuffer(vertexBuffer);
		vulkan.DestroyBuffer(indexBuffer);
	}
	void Draw(vk::CommandBuffer commands) const
	{
		commands.bindVertexBuffers(0, { vertexBuffer }, { 0 });
		if (indexBuffer)
		{
			commands.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
			commands.drawIndexed(elementCount, 1, 0, 0, 0);
		}
		else
		{
			commands.draw(elementCount, 1, 0, 0);
		}
	}
};

template <typename Constants>
struct UniformBuffer
{
	Constants data;
	vk::Buffer buffer;
	VulkanWrapper::ShaderUniform uniform;

	Constants* operator ->() { return &data; }

	void Initialize(VulkanWrapper& vulkan)
	{
		buffer = vulkan.CreateUniformBuffer(sizeof(Constants));
		uniform = vulkan.CreateUniformForBuffer(Constants::kBinding, buffer);
	}
	void Cleanup(VulkanWrapper& vulkan)
	{
		vulkan.DestroyUniform(uniform);
		vulkan.DestroyBuffer(buffer);
	}
	void Update(vk::CommandBuffer commands)
	{
		commands.updateBuffer(buffer, 0, sizeof(data), &data);
	}
};

struct Material
{
	enum OptionFlags
	{
		eNone         = 0x0,
		eLit          = 0x01 << 0,
		eCastShadows  = 0x01 << 1,
		eCatchShadows = 0x01 << 2, // ?
	};

	glm::vec4 color = {};
	uint32_t options = eNone;
	float shadowBias = 0.005f;
};

struct Light
{
	alignas(16) glm::vec3 position = {};
	alignas(16) glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	alignas(16) glm::mat4 viewProj = glm::identity<glm::mat4>();
};

struct SceneConstants
{
	static constexpr uint32_t kBinding = 0;
	alignas(16) glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
	alignas(16) glm::mat4 viewMatrix = glm::identity<glm::mat4>();

	Light light0;
};

struct MeshPushConstants
{
	glm::mat4 objectMatrix = glm::identity<glm::mat4>();
	Material material;
};

struct ShadowPushConstants
{
	glm::mat4 light0Matrix = glm::identity<glm::mat4>();
	glm::mat4 objectMatrix = glm::identity<glm::mat4>();
};

DrawableMesh meshCube, meshFloor, meshPyramid;
Material materialCube, materialFloor;
Private::Transform transformFloor, transformLight;
std::array<Private::Transform, 10> transformCube;
std::array<Private::Transform, 10> transformPyramid;
UniformBuffer<SceneConstants> constantsScene;
VulkanWrapper::RenderTarget targetShadowMap;
VulkanWrapper::ShaderUniform uniformShadowMap;
vk::Sampler samplerShadowMap, imguiSamplerShadowMapVis;
GraphicsPipelinePtr pipelineMesh, pipelineShadows;
float lightFOV = 90.0f;
float lightNear = 1.0f;
float lightFar = 100.0f;
float depthBiasConstant = 1.25f;
float depthBiasSlope = 1.75f;
const vk::Extent2D lightMapDimensions = { 1024, 1024 };
vk::DescriptorSet imguiLightShadowMapDS;
vk::ImageView imguiLightShadowMapView;

void ImGuiTransform(Private::Transform& transform)
{
	glm::vec3 position = transform.GetPosition();
	glm::vec3 rotation = glm::degrees(transform.GetRotation());
	glm::vec3 scale = transform.GetScale();

	ImGui::PushID(&transform);
	ImGui::DragFloat3("Position", glm::value_ptr(position), 0.25f);
	ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.25f);
	ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f);
	ImGui::PopID();

	transform.SetPosition(position);
	transform.SetRotation(glm::radians(rotation));
	transform.SetScale(scale);
}

void UnloadScene()
{
	meshCube.Cleanup(vulkan);
	meshFloor.Cleanup(vulkan);
	meshPyramid.Cleanup(vulkan);
	vulkan.DestroyPipeline(pipelineMesh);
	vulkan.DestroyPipeline(pipelineShadows);
	ImGui_ImplVulkan_RemoveTexture(imguiLightShadowMapDS);
	vulkan.GetDevice().destroyImageView(imguiLightShadowMapView);
	vulkan.GetDevice().destroySampler(samplerShadowMap);
	vulkan.GetDevice().destroySampler(imguiSamplerShadowMapVis);
	vulkan.DestroyUniform(uniformShadowMap);
	vulkan.DestroyRenderTarget(targetShadowMap);
	constantsScene.Cleanup(vulkan);
}

void LoadScene()
{
	camera.SetAspect(vulkan.GetDisplayViewport().width / vulkan.GetDisplayViewport().height);
	camera.SetPosition({ 2.0f, 3.0f, -15.0f });

	constantsScene.Initialize(vulkan);
	samplerShadowMap = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
			.setMinFilter(vk::Filter::eNearest)
			.setMagFilter(vk::Filter::eNearest)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
			//.setCompareEnable(true)
			//.setCompareOp(vk::CompareOp::eLessOrEqual)
	);
	imguiSamplerShadowMapVis = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
			.setMinFilter(vk::Filter::eNearest)
			.setMagFilter(vk::Filter::eNearest)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
	);
	targetShadowMap = vulkan.CreateRenderTarget(lightMapDimensions, vk::Format::eUndefined, vk::Format::eD16Unorm);
	uniformShadowMap = vulkan.CreateUniformForImage(1, targetShadowMap.views[0], samplerShadowMap);

	imguiLightShadowMapView = vulkan.GetDevice().createImageView(
		vk::ImageViewCreateInfo()
			.setImage(targetShadowMap.images[0])
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

	imguiLightShadowMapDS = ImGui_ImplVulkan_AddTexture(imguiSamplerShadowMapVis, imguiLightShadowMapView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	const MeshData kFloor =
	{
		.vertices =
		{
			{ .position = { -15.0f, 0.0f, -10.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .texcoord0 = { 0.0f, 4.0f } },
			{ .position = { -15.0f, 0.0f,  10.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .texcoord0 = { 0.0f, 0.0f } },
			{ .position = {  15.0f, 0.0f,  10.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .texcoord0 = { 6.0f, 0.0f } },
			{ .position = {  15.0f, 0.0f, -10.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .texcoord0 = { 6.0f, 4.0f } },
		},
		.indices =
		{
			0, 2, 1,
			2, 0, 3,
		},
	};

	meshCube = DrawableMesh::Create(Shapes::kTexturedCube, vulkan);
	meshFloor = DrawableMesh::Create(kFloor, vulkan);
	meshPyramid = DrawableMesh::Create(Shapes::kPyramid, vulkan);

	materialCube.color = { 0.0f, 1.0f, 0.0f, 1.0f };
	materialCube.options = Material::eLit | Material::eCastShadows;
	materialFloor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialFloor.options = Material::eLit | Material::eCatchShadows;

	for (auto& transform : transformCube)
	{
		auto pos = glm::diskRand(8.0f);
		float y = 0.5f; //glm::linearRand(-1.0f, 1.0f);
		transform.SetPosition({ pos.x, y, pos.y });
	}

	for (auto& transform : transformPyramid)
	{
		auto pos = glm::diskRand(8.0f);
		float y = 0.5f; //glm::linearRand(-1.0f, 1.0f);
		transform.SetPosition({ pos.x, y, pos.y });
	}

	//transformCube.SetScale({ 1, 1, 1 });
	transformLight.SetPosition({ -5.0f, 1.75f, 3.75f });
	transformLight.SetRotation({ glm::radians(45.0f), 0.0f, 0.0f });

	pipelineMesh = vulkan.CreatePipeline({
		.vertexBindings =
		{
			vk::VertexInputBindingDescription()
				.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(MeshData::Vertex))
		},
		.vertexAttributes =
		{
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshData::Vertex, MeshData::Vertex::position)),
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(1)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshData::Vertex, MeshData::Vertex::normal))
		},
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				.setOffset(0)
				.setSize(sizeof(MeshPushConstants))
		},
		.shaderUniforms =
		{
			constantsScene.uniform,
			uniformShadowMap,
		},
		.shaderStages = LoadShaders("mesh.vert", "mesh.frag"),
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthTestEnable = true,
		.depthCompareOp = vk::CompareOp::eLess,
		.depthWriteEnable = true,
		.colorBlendStates =
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
		},
	});

	pipelineShadows = vulkan.CreatePipeline({
		.vertexBindings =
		{
			vk::VertexInputBindingDescription()
				.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(MeshData::Vertex))
		},
		.vertexAttributes =
		{
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshData::Vertex, MeshData::Vertex::position))
		},
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				.setOffset(0)
				.setSize(sizeof(ShadowPushConstants))
		},
		.shaderStages = LoadShaders("shadow.vert", "shadow.frag"),
		.cullMode = vk::CullModeFlagBits::eFront,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthTestEnable = true,
		.depthCompareOp = vk::CompareOp::eLessOrEqual,
		.depthWriteEnable = true,
		.depthBiasEnable = true,
		.colorBlendStates =
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
		},
		.viewports =
		{
			vk::Viewport(0, 0, lightMapDimensions.width, lightMapDimensions.height, 0, 1)
		},
		.renderPass = targetShadowMap.renderPass,
		.dynamicStates =
		{
			vk::DynamicState::eDepthBias,
		},
	});
}

void PreDrawScene(vk::CommandBuffer& commands)
{
	const glm::mat4& lightMatrix = transformLight.GetGlobalTransform();

	const glm::mat4 lightProjMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, lightNear, lightFar);
	const glm::mat4 lightViewMatrix = glm::lookAt(
		glm::xyz(glm::column(lightMatrix, 3)),
		{ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }
		//glm::xyz(glm::column(lightMatrix, 3) + lightMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)),
		//glm::xyz(lightMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f))
	);
	constantsScene->light0.position = transformLight.GetGlobalPosition();
	constantsScene->light0.viewProj = lightProjMatrix * lightViewMatrix;

	constantsScene->projectionMatrix = camera.GetProjectionMatrix();
	constantsScene->viewMatrix = camera.GetViewMatrix();
	constantsScene.Update(commands);
}

void DrawShadows(vk::CommandBuffer& commands)
{
	constexpr vk::ClearValue clearValues[] =
	{
		vk::ClearDepthStencilValue{ 1.0f, 0 },
	};
	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(targetShadowMap.renderPass);
	rpbi.setFramebuffer(targetShadowMap.framebuffer);
	rpbi.setClearValues(clearValues);
	rpbi.setRenderArea({ { 0, 0 }, lightMapDimensions });
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	commands.setDepthBias(depthBiasConstant, 0.0f, depthBiasSlope);

	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineShadows->GetPipeline());

	ShadowPushConstants pushConstants;
	pushConstants.light0Matrix = constantsScene->light0.viewProj;

	for (auto& transform : transformCube)
	{
		pushConstants.objectMatrix = transform.GetGlobalTransform();
		commands.pushConstants(pipelineShadows->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		meshCube.Draw(commands);
	}

	for (auto& transform : transformPyramid)
	{
		pushConstants.objectMatrix = transform.GetGlobalTransform();
		commands.pushConstants(pipelineShadows->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		meshPyramid.Draw(commands);
	}

	commands.endRenderPass();
}

void DrawScene(vk::CommandBuffer& commands, vk::Framebuffer& framebuffer)
{
	MeshPushConstants pushConstants;

	commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineMesh->GetPipeline());
	commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineMesh->GetLayout(), 0, { constantsScene.uniform, uniformShadowMap }, {});

	pushConstants.objectMatrix = transformFloor.GetGlobalTransform();
	pushConstants.material = materialFloor;
	commands.pushConstants(pipelineMesh->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
	meshFloor.Draw(commands);

	pushConstants.material = materialCube;
	for (auto& transform : transformCube)
	{
		pushConstants.objectMatrix = transform.GetGlobalTransform();
		commands.pushConstants(pipelineMesh->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		meshCube.Draw(commands);
	}
	for (auto& transform : transformPyramid)
	{
		pushConstants.objectMatrix = transform.GetGlobalTransform();
		commands.pushConstants(pipelineMesh->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		meshPyramid.Draw(commands);
	}

	pushConstants.objectMatrix = transformLight.GetGlobalTransform() * glm::scale(glm::identity<glm::mat4>(), { 0.1f, 0.1f, 0.1f });
	pushConstants.material.color = glm::vec4(constantsScene->light0.color, 1.0f);
	pushConstants.material.options = Material::eNone;
	commands.pushConstants(pipelineMesh->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
	meshCube.Draw(commands);
}

void AnimateScene(float tickDelta)
{
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Cube");
			ImGuiTransform(transformLight);
			ImGui::ColorEdit3("Color", glm::value_ptr(constantsScene->light0.color));
			ImGui::DragFloat("FOV", &lightFOV, 1.0f, 1.0f, 360.0f);
			glm::vec2 range = { lightNear, lightFar };
			ImGui::DragFloat2("Range", glm::value_ptr(range));
			ImGui::DragFloat("Depth Bias Constant", &depthBiasConstant, 0.01f);
			ImGui::DragFloat("Depth Bias Slope", &depthBiasSlope, 0.01f);
			lightNear = range.x; lightFar = range.y;
			if (ImGui::CollapsingHeader("Shadow Map"))
			{
				ImGui::Image(imguiLightShadowMapDS, { 256, 256 });
			}
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Cubes"))
		{
			ImGui::PushID("Cubes");
			uint32_t index = 0;
			for (auto& transform : transformCube)
			{
				static std::string label;
				label = "Cube " + std::to_string(index);
				index++;

				if (ImGui::CollapsingHeader(label.c_str()))
				{
					ImGuiTransform(transform);
				}
			}
			ImGui::ColorEdit4("Color", glm::value_ptr(materialCube.color));
			ImGui::CheckboxFlags("Lit", &materialCube.options, Material::eLit);
			ImGui::CheckboxFlags("Shadowed", &materialCube.options, Material::eCatchShadows);
			ImGui::DragFloat("Shadow Bias", &materialCube.shadowBias, 0.001f);
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Floor", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Floor");
			ImGuiTransform(transformFloor);
			ImGui::ColorEdit4("Color", glm::value_ptr(materialFloor.color));
			ImGui::CheckboxFlags("Lit", &materialFloor.options, Material::eLit);
			ImGui::CheckboxFlags("Shadowed", &materialFloor.options, Material::eCatchShadows);
			ImGui::DragFloat("Shadow Bias", &materialFloor.shadowBias, 0.001f);
			ImGui::PopID();
		}
	}
	ImGui::End();
	
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

	LoadScene();

	return true;
}

void AppShutdown()
{
	vulkan.GetDevice().waitIdle();
	UnloadScene();
	debugGrid.Cleanup();
	ImGuiShutdown();
	shaderLib.Shutdown();
	vulkan.Shutdown();
}

void AppOnTick(float tickDelta)
{
	ImGuiNewFrame();

	cameraController.Tick(tickDelta);
	AnimateScene(tickDelta);

	vk::CommandBuffer commands;
	vk::Framebuffer framebuffer;
	if (!vulkan.BeginFrame(commands, framebuffer))
		return;

	PreDrawScene(commands);

	DrawShadows(commands);

	constexpr vk::ClearValue clearValues[] =
	{
		vk::ClearColorValue{ 0.01f, 0.01f, 0.01f, 0.0f },
		vk::ClearDepthStencilValue{ 1.0f, 0 },
	};
	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(vulkan.GetDisplayRenderPass());
	rpbi.setFramebuffer(framebuffer);
	rpbi.setClearValues(clearValues);
	rpbi.setRenderArea({ { 0, 0 }, vulkan.GetSwapchain().extent });
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	DrawScene(commands, framebuffer);

	debugGrid.Draw(commands, camera.GetProjectionMatrix(), camera.GetViewMatrix());
	ImGuiRender(commands);

	commands.endRenderPass();
	vulkan.EndFrame();
}

void AppOnEvent(const SDL_Event& event)
{
	ImGuiOnEvent(event);
	cameraController.OnEvent(event);
}
#endif