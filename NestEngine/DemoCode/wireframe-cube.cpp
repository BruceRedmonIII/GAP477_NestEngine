#if _DEMO // Gets rid of the text while keeping the syntax highlighting. This code is never planned to be used, just here for reference
#include <gap-311/VulkanWrapper.hpp>
#include <gap-311/VulkanShaderLibrary.hpp>

#include <filesystem>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace GAP311;

using TickFunc = std::function<void(float)>;
using ShutdownFunc = std::function<void()>;

bool ApplicationMain(SDL_Rect windowRect, SDL_Window* pWindow, VulkanWrapper& vulkan, VulkanShaderLibrary& shaderLib,
	std::function<void(ShutdownFunc, TickFunc)>& runApplication)
{
	//
	//   4----5
	//  /|   /|
	// 0----1 |
	// | 7--|-6    y z
	// |/   |/     |/
	// 3----2      +--x
	//
	const glm::vec3 positions[] =
	{
		{ -0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },
		{  0.5f, -0.5f,  0.5f },
		{ -0.5f, -0.5f,  0.5f },
	};

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

	const uint32_t indexCount = _countof(indices);

	vk::Buffer positionsBuffer = vulkan.CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer, sizeof(positions), positions);
	vk::Buffer indexBuffer = vulkan.CreateIndexBuffer(sizeof(indices), indices);

	struct Uniforms
	{
		glm::mat4 objectMatrix = glm::identity<glm::mat4>();
		glm::mat4 viewMatrix = glm::identity<glm::mat4>();
		glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
	} uniforms;

	uniforms.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)windowRect.w / windowRect.h, 0.1f, 512.0f);
	uniforms.viewMatrix = glm::lookAt(
		glm::vec3{ 0.0f, 2.0f, -2.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f }
	);
	uniforms.projectionMatrix[1][1] *= -1.0f;

	const GraphicsPipelineConfig pipelineConfig =
	{
		.vertexBindings =
		{
			vk::VertexInputBindingDescription()
				.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(positions[0])),
		},
		.vertexAttributes =
		{
			vk::VertexInputAttributeDescription()
				.setLocation(0)
				.setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(0),
		},
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setOffset(0)
				.setSize(sizeof(Uniforms))
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName("main")
				.setModule(shaderLib.GetModule("vertex_proj.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName("main")
				.setModule(shaderLib.GetModule("vertex_color.frag"))
		},
		.polygonMode = vk::PolygonMode::eLine,
		//.cullMode = vk::CullModeFlagBits::eBack,
		//.frontFace = vk::FrontFace::eCounterClockwise,
		//.depthTestEnable = false,
		//.depthCompareOp = vk::CompareOp::eLess,
		//.depthWriteEnable = false,
	};
	auto pipeline = vulkan.CreatePipeline(pipelineConfig);

	runApplication(
		[&]()
		{
			vulkan.DestroyPipeline(pipeline);

			if (positionsBuffer)
				vulkan.DestroyBuffer(positionsBuffer);
			if (indexBuffer)
				vulkan.DestroyBuffer(indexBuffer);
		},
		[&](float tickDelta)
		{
			static float rotation = 0.0f;
			rotation += tickDelta;
			if (rotation > glm::two_pi<float>())
				rotation -= glm::two_pi<float>();

			uniforms.objectMatrix = glm::rotate(glm::identity<glm::mat4>(), rotation, glm::vec3{ 0.0f, 1.0f, 0.0f });

			vk::CommandBuffer commands;
			vk::Framebuffer framebuffer;
			if (!vulkan.BeginFrame(commands, framebuffer))
				return;

			vk::ClearValue clearValues[] =
			{
				vk::ClearColorValue(0.05f, 0.05f, 0.05f, 0.0f),
				vk::ClearDepthStencilValue( 1.0f, 0 ),
			};

			vk::RenderPassBeginInfo rpbi{};
			rpbi.setRenderPass(vulkan.GetDisplayRenderPass());
			rpbi.setFramebuffer(framebuffer);
			rpbi.setClearValues(clearValues);
			rpbi.setRenderArea(vk::Rect2D({ 0, 0 }, { static_cast<uint32_t>(windowRect.w), static_cast<uint32_t>(windowRect.h) }));
			commands.beginRenderPass(rpbi, vk::SubpassContents::eInline);

			commands.pushConstants(pipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(uniforms), &uniforms);
			commands.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
			commands.bindVertexBuffers(0, { positionsBuffer }, { 0 });
			commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
			commands.drawIndexed(indexCount, 1, 0, 0, 0);

			commands.endRenderPass();
			vulkan.EndFrame();
		}
	);

	return true;
}

int main(int argc, char* argv[])
{
	const char* appName = argv[0];

	if (0 > SDL_Init(SDL_INIT_VIDEO))
		return -1;

	SDL_Rect windowRect = { SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720 };

	SDL_Window* pSDLWindow = SDL_CreateWindow(appName, windowRect.x, windowRect.y, windowRect.w, windowRect.h, SDL_WINDOW_VULKAN);
	if (!pSDLWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

#ifdef _DEBUG
	constexpr bool enableBreakOnError = true;
#else
	constexpr bool enableBreakOnError = false;
#endif

	VulkanWrapper vulkan;
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
	options.debugLogFunc = VulkanWrapper::VulkanLogToSDL(enableBreakOnError);
	options.enableDepthStencilBuffer = true;
	if (!vulkan.Initialize(appName, VulkanWrapper::CreateVulkanSurfaceSDL(pSDLWindow), options))
	{
		SDL_Log("Failed initializing VulkanWrapper.");
		SDL_DestroyWindow(pSDLWindow);
		SDL_Quit();
		return -1;
	}

	VulkanShaderLibrary shaderLib;
	VulkanShaderLibrary::ConfigOptions shaderOptions{};
	shaderOptions.logMessage = [](const char* msg)
		{
			SDL_Log("ShaderLibrary: %s", msg);
		};
	shaderOptions.searchDirs = {
		std::filesystem::current_path().string(), // working dir
		std::filesystem::path(argv[0]).parent_path().string(), // exe dir
		std::filesystem::relative(std::filesystem::path(__FILE__).parent_path()).string(), // source file dir
	};
	if (!shaderLib.Initialize(vulkan.GetDevice(), shaderOptions))
	{
		SDL_Log("Failed initializing VulkanShaderLibrary.");
		vulkan.Shutdown();
		SDL_DestroyWindow(pSDLWindow);
		SDL_Quit();
		return -1;
	}

	std::function runApp = [&](ShutdownFunc appShutdown, TickFunc appTick)
		{
			const uint64_t timerFreq = SDL_GetPerformanceFrequency();
			uint64_t timerLastTicks = SDL_GetPerformanceCounter();
			float deltaTime = 0;
			
			bool shutdownRequested = false;
			while (!shutdownRequested)
			{
				SDL_Event event;
				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_QUIT)
						shutdownRequested = true;
				}

				appTick(deltaTime);

				uint64_t timerNow = SDL_GetPerformanceCounter();
				deltaTime = float(double(timerNow - timerLastTicks) / timerFreq);
				timerLastTicks = timerNow;
			}

			vulkan.GetDevice().waitIdle();
			appShutdown();
		};

	bool result = ApplicationMain(windowRect, pSDLWindow, vulkan, shaderLib, runApp);
	if (!result)
	{
		SDL_Log("Application indicated failure.");
	}

	shaderLib.Shutdown();
	vulkan.Shutdown();
	SDL_DestroyWindow(pSDLWindow);
	SDL_Quit();
	return result ? 0 : -1;
}
#endif