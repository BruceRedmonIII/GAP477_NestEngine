#if _DEMO // Gets rid of the text while keeping the syntax highlighting. This code is never planned to be used, just here for reference
#include <gap-311/VulkanWrapper.hpp>
#include <gap-311/VulkanShaderLibrary.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <SDL_Image.h>

#include <imgui.h>
#include <private/DebugGrid.hpp>
#include <private/Utils.hpp>

using namespace GAP311;

extern bool ImGuiInitialize(SDL_Window*, VulkanWrapper&);
extern void ImGuiShutdown();
extern void ImGuiNewFrame();
extern void ImGuiRender(vk::CommandBuffer& commands);
extern void ImGuiOnEvent(const SDL_Event& event);

extern struct Camera* CameraCreate();
extern void CameraDestroy(Camera* pCamera);
extern void CameraGetViewMatrix(Camera* pCamera, glm::mat4* pMatrix);
extern void CameraUpdate(Camera* pCamera, float tickDelta);
extern void CameraOnEvent(Camera* pCamera, const SDL_Event& event);

struct MeshVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord0;
};

struct MeshData
{
	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;
	uint32_t indexCount = 0;
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

	glm::vec3 lightPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
	glm::vec3 lightAttenuation = { 1.0f, 1.0f, 0.0f };
};

VulkanWrapper vulkan;
VulkanShaderLibrary shaderLib;
std::shared_ptr<VulkanWrapper::GraphicsPipeline> meshPipeline;
std::shared_ptr<VulkanWrapper::GraphicsPipeline> skyboxPipeline;

MeshData cubeMesh;
std::vector<glm::mat4> cubeTransforms;

struct Camera* camera = nullptr;
CameraConstants cameraConstants;
VulkanWrapper::ShaderUniform cameraUniform;
vk::Buffer cameraConstantsBuffer;

Private::DebugGrid debugGrid;
std::vector<std::filesystem::path> assetSearchDirs;

VulkanWrapper::ShaderUniform texture0Uniform;
vk::Image texture0Image;
vk::ImageView texture0ImageView;
vk::Sampler texture0Sampler;

VulkanWrapper::ShaderUniform texture1Uniform;
vk::Image texture1Image;
vk::ImageView texture1ImageView;
vk::Sampler texture1Sampler;

VulkanWrapper::ShaderUniform textureCubeUniform;
vk::Image textureCubeImage;
vk::ImageView textureCubeImageView;
vk::Sampler textureCubeSampler;

SDL_Surface* LoadCompatibleImage(std::filesystem::path imageFile, Uint32 pixelFormat = SDL_PIXELFORMAT_ABGR8888)
{
	imageFile = Private::ResolveFile(imageFile, assetSearchDirs);
	SDL_Surface* surface = IMG_Load(imageFile.string().c_str());
	if (!surface)
		return nullptr;

	if (surface->format->format != pixelFormat)
	{
		SDL_Surface* newFormat = SDL_ConvertSurfaceFormat(surface, pixelFormat, 0);
		if (newFormat)
		{
			SDL_FreeSurface(surface);
			surface = newFormat;
		}
	}

	return surface;
}

std::tuple<vk::Buffer, vk::Buffer, uint32_t> LoadPlaneAsset()
{
	//
	//    2-----3
	//   / \   /
	//  /   \ /
	// 0-----1
	//
	MeshVertex vertices[] =
	{
		{ { -0.5f,  0.0f, -0.5f }, {}, { 0.0f, 1.0f, } },
		{ {  0.5f,  0.0f, -0.5f }, {}, { 1.0f, 1.0f, } },
		{ { -0.5f,  0.0f,  0.5f }, {}, { 0.0f, 0.0f, } },
		{ {  0.5f,  0.0f,  0.5f }, {}, { 1.0f, 0.0f, } },
	};

	// Generate normals from position
	for (auto& vertex : vertices)
	{
		vertex.normal = glm::normalize(vertex.position);
	}

	const uint32_t indices[] =
	{
		0, 1, 2,
		1, 3, 2,
	};

	auto indexBuffer = vulkan.CreateIndexBuffer(sizeof(indices), indices);
	auto vertexBuffer = vulkan.CreateVertexBuffer(sizeof(vertices), vertices);
	return { vertexBuffer, indexBuffer, _countof(indices) };
}

std::tuple<vk::Buffer, vk::Buffer, uint32_t> LoadCubeAsset()
{
	//
	//   4----5
	//  /|   /|
	// 0----1 |
	// | 7--|-6    y z
	// |/   |/     |/
	// 3----2      +--x
	//
	MeshVertex vertices[] =
	{
		{ { -0.5f,  0.5f, -0.5f }, {}, { } },
		{ {  0.5f,  0.5f, -0.5f }, {}, { } },
		{ {  0.5f, -0.5f, -0.5f }, {}, { } },
		{ { -0.5f, -0.5f, -0.5f }, {}, { } },
		{ { -0.5f,  0.5f,  0.5f }, {}, { } },
		{ {  0.5f,  0.5f,  0.5f }, {}, { } },
		{ {  0.5f, -0.5f,  0.5f }, {}, { } },
		{ { -0.5f, -0.5f,  0.5f }, {}, { } },
	};

	for (auto& vertex : vertices)
	{
		vertex.normal = glm::normalize(vertex.position);
		vertex.texcoord0.x = vertex.position.x + 0.5f;
		vertex.texcoord0.y = vertex.position.y + 0.5f;
	}

	const uint32_t indices[] =
	{
		0, 3, 2, // Front
		0, 2, 1,
		4, 5, 7, // Rear
		5, 6, 7,
		1, 2, 6, // Right
		5, 1, 6,
		0, 4, 7, // Left
		0, 7, 3,
		5, 4, 0, // Top
		5, 0, 1,
		7, 6, 2, // Bottom
		7, 2, 3,
	};

	auto indexBuffer = vulkan.CreateIndexBuffer(sizeof(indices), indices);
	auto vertexBuffer = vulkan.CreateVertexBuffer(sizeof(vertices), vertices);
	return { vertexBuffer, indexBuffer, _countof(indices) };
}

void LoadTexture0()
{
	texture0Sampler = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setMinFilter(vk::Filter::eLinear)
			.setMagFilter(vk::Filter::eLinear)
	);

	SDL_Surface* color = LoadCompatibleImage(Private::ResolveFile("bricks-color.png", assetSearchDirs));

	std::tie(texture0Image, texture0ImageView) = vulkan.CreateTexture2DImageAndView(
		{ static_cast<uint32_t>(color->w), static_cast<uint32_t>(color->h) },
		VulkanWrapper::VulkanFormatFromSDLFormat(color->format->format),
		color->pixels, color->w * color->h * color->format->BytesPerPixel
	);

	texture0Uniform = vulkan.CreateUniformForImage(1, texture0ImageView, texture0Sampler);
}

void LoadTexture1()
{
	texture1Sampler = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setMinFilter(vk::Filter::eLinear)
			.setMagFilter(vk::Filter::eLinear)
	);

	SDL_Surface* color = LoadCompatibleImage(Private::ResolveFile("bricks-normal.png", assetSearchDirs));

	std::tie(texture1Image, texture1ImageView) = vulkan.CreateTexture2DImageAndView(
		{ static_cast<uint32_t>(color->w), static_cast<uint32_t>(color->h) },
		VulkanWrapper::VulkanFormatFromSDLFormat(color->format->format),
		color->pixels, color->w * color->h * color->format->BytesPerPixel
	);

	texture1Uniform = vulkan.CreateUniformForImage(2, texture1ImageView, texture1Sampler);
}

void LoadTextureCube()
{
	textureCubeSampler = vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setMinFilter(vk::Filter::eLinear)
			.setMagFilter(vk::Filter::eLinear)
	);

	const char* imagePaths[] =
	{
		"miramar_ft.png", // Front
		"miramar_bk.png", // Back
		"miramar_up.png", // Up
		"miramar_dn.png", // Down
		"miramar_rt.png", // Right
		"miramar_lf.png", // Left
	};

	std::vector<uint8_t> imageBytes;

	for (const auto& path : imagePaths)
	{
		SDL_Surface* side = LoadCompatibleImage(Private::ResolveFile(path, assetSearchDirs));

		const uint8_t* start = static_cast<const uint8_t*>(side->pixels);
		const uint8_t* end = &static_cast<const uint8_t*>(side->pixels)[side->w * side->h * side->format->BytesPerPixel];

		imageBytes.insert(imageBytes.end(), start, end);
	}

	std::tie(textureCubeImage, textureCubeImageView) = vulkan.CreateCubemapImageAndView(
		{ 1024, 1024 },
		VulkanWrapper::VulkanFormatFromSDLFormat(SDL_PIXELFORMAT_ABGR8888),
		imageBytes.data(), imageBytes.size()
	);

	textureCubeUniform = vulkan.CreateUniformForImage(3, textureCubeImageView, textureCubeSampler);
}

bool AppStartup(SDL_Window* pWindow, const char* exePath)
{
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

	assetSearchDirs =
	{
		std::filesystem::current_path().string(), // working dir
		std::filesystem::path(exePath).parent_path().string(), // exe dir
		std::filesystem::relative(std::filesystem::path(__FILE__).parent_path()).string(), // source file dir
	};

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
	std::transform(assetSearchDirs.begin(), assetSearchDirs.end(),
		std::back_inserter(shaderOptions.searchDirs), [](const std::filesystem::path& p) { return p.string(); });
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

	LoadTexture0();
	LoadTexture1();
	LoadTextureCube();

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
			vk::VertexInputAttributeDescription()
				.setLocation(1)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
			vk::VertexInputAttributeDescription()
				.setLocation(2)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::texcoord0)),
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
			texture0Uniform,
			texture1Uniform,
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName("main")
				.setModule(shaderLib.GetModule("mesh.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName("main")
				.setModule(shaderLib.GetModule("mesh.frag"))
		},
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

	skyboxPipeline = vulkan.CreatePipeline(
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
			vk::VertexInputAttributeDescription()
				.setLocation(1)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
			vk::VertexInputAttributeDescription()
				.setLocation(2)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::texcoord0)),
		},
		.shaderUniforms =
		{
			cameraUniform,
			textureCubeUniform,
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName("main")
				.setModule(shaderLib.GetModule("skybox.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName("main")
				.setModule(shaderLib.GetModule("skybox.frag"))
		},
	});

	std::tie(cubeMesh.vertexBuffer, cubeMesh.indexBuffer, cubeMesh.indexCount) = LoadCubeAsset();

	return true;
}

void AppShutdown()
{
	vulkan.GetDevice().waitIdle();

	debugGrid.Cleanup();

	CameraDestroy(camera);
	vulkan.DestroyUniform(cameraUniform);
	vulkan.DestroyBuffer(cameraConstantsBuffer);

	vulkan.DestroyPipeline(meshPipeline);
	vulkan.DestroyBuffer(cubeMesh.indexBuffer);
	vulkan.DestroyBuffer(cubeMesh.vertexBuffer);

	vulkan.DestroyUniform(texture0Uniform);
	vulkan.DestroyImageAndView(texture0Image, texture0ImageView);
	vulkan.GetDevice().destroySampler(texture0Sampler);

	vulkan.DestroyUniform(texture1Uniform);
	vulkan.DestroyImageAndView(texture1Image, texture1ImageView);
	vulkan.GetDevice().destroySampler(texture1Sampler);

	vulkan.DestroyUniform(textureCubeUniform);
	vulkan.DestroyImageAndView(textureCubeImage, textureCubeImageView);
	vulkan.GetDevice().destroySampler(textureCubeSampler);

	ImGuiShutdown();
	shaderLib.Shutdown();
	vulkan.Shutdown();

	IMG_Quit();
}

void AppOnTick(float tickDelta)
{
	ImGuiNewFrame();

	CameraUpdate(camera, tickDelta);
	CameraGetViewMatrix(camera, &cameraConstants.viewMatrix);

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::CollapsingHeader("Lighting");
		ImGui::DragFloat3("Light Position", glm::value_ptr(cameraConstants.lightPosition));
		ImGui::ColorEdit3("Light Color", glm::value_ptr(cameraConstants.lightColor));
		ImGui::DragFloat3("Light Attenuation", glm::value_ptr(cameraConstants.lightAttenuation));

	}
	ImGui::End();

	vk::CommandBuffer commands;
	vk::Framebuffer framebuffer;
	if (!vulkan.BeginFrame(commands, framebuffer))
		return;

	// UPDATE!
	commands.updateBuffer(cameraConstantsBuffer, 0, sizeof(cameraConstants), &cameraConstants);

	vk::ClearValue clearValues[] =
	{
		vk::ClearColorValue(0.05f, 0.05f, 0.05f, 0.0f),
		vk::ClearDepthStencilValue(1.0f, 0),
	};

	vk::RenderPassBeginInfo rpbi{};
	rpbi.setRenderPass(vulkan.GetDisplayRenderPass());
	rpbi.setFramebuffer(framebuffer);
	rpbi.setClearValues(clearValues);
	rpbi.setRenderArea({ { 0, 0 }, vulkan.GetSwapchain().extent });
	commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	// Skybox
	{
		commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxPipeline->GetLayout(), 0, { cameraUniform, textureCubeUniform }, { });
		commands.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxPipeline->GetPipeline());
		commands.bindIndexBuffer(cubeMesh.indexBuffer, 0, vk::IndexType::eUint32);
		commands.bindVertexBuffers(0, { cubeMesh.vertexBuffer }, { 0 });
		commands.drawIndexed(cubeMesh.indexCount, 1, 0, 0, 0);
	}

	{
		PushConstants pushConstants;
		pushConstants.objectMatrix = glm::identity<glm::mat4>();
		pushConstants.objectColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		commands.pushConstants(meshPipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(pushConstants), &pushConstants);
		commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshPipeline->GetLayout(), 0, { cameraUniform, texture0Uniform, texture1Uniform }, { });
		commands.bindPipeline(vk::PipelineBindPoint::eGraphics, meshPipeline->GetPipeline());
		commands.bindIndexBuffer(cubeMesh.indexBuffer, 0, vk::IndexType::eUint32);
		commands.bindVertexBuffers(0, { cubeMesh.vertexBuffer }, { 0 });
		commands.drawIndexed(cubeMesh.indexCount, 1, 0, 0, 0);
	}

	debugGrid.Draw(commands, cameraConstants.projectionMatrix, cameraConstants.viewMatrix);
	ImGuiRender(commands);

	commands.endRenderPass();
	vulkan.EndFrame();
}

void AppOnEvent(const SDL_Event& event)
{
	ImGuiOnEvent(event);
	CameraOnEvent(camera, event);
}

#endif
