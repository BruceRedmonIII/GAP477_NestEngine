#pragma once

///
/// VULKAN WRAPPER FOR GAP-311
/// Written by Josh Stefanski.
///
/// This file contains a wrapper around some core Vulkan objects and
/// some related helpers to make working with the Vulkan API a bit less
/// verbose and confusing. It is not intended to be a feature-complete
/// abstraction layer, instead just a light wrapper. For many advanced
/// usage cases it should not get in your way / you are free to manually
/// take over. The method-chaining pattern of member initialization
/// is used extensively in this code as coupled with Vulkan-Hpp it
/// can improve the readability of the larger structs and leverage some
///	features to handle simpler code.
///

/// Include the official Vulkan C++ header.
/// This is a C++-friendly wrapper around the C API in vulkan.h
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_hash.hpp>

/// VkBootstrap further helps with abstracting the Vulkan
/// initialization process
#include <VkBootstrap.h>

#include <tuple>
#include <optional>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

/// Detect and enable support for integration with other libraries.
///
/// These features can be forcefully enabled or disabled by
/// defining the GAP311_ENABLE_ value before including this file
/// or (ideally) as part of the project properties.
///
/// Example:
///   #define GAP311_ENABLE_SDL 0 // force disable SDL
///   #define GAP311_ENABLE_SFML 1 // force enable SFML
///   #include <VulkanHelpers.hpp>
#if __has_include(<SDL_vulkan.h>) && !defined(GAP311_ENABLE_SDL)
#   define GAP311_ENABLE_SDL 1
#endif
#if __has_include(<GLFW/glfw3.h>) && !defined(GAP311_ENABLE_GLFW)
#   define GAP311_ENABLE_GLFW 1
#endif
#if __has_include(<SFML/Window.hpp>) && !defined(GAP311_ENABLE_SFML)
#   define GAP311_ENABLE_SFML 1
#endif
#if __has_include(<Windows.h>) && !defined(GAP311_ENABLE_WIN32)
#   define GAP311_ENABLE_WIN32 0 // Default WIN32 off (use unlikely)
#endif

/// Forward declarations for integrations
#if GAP311_ENABLE_SDL
#	include <SDL.h>
#   include <SDL_vulkan.h>
#endif
#if GAP311_ENABLE_GLFW
#   include <GLFW/glfw3.h>
#endif
#if GAP311_ENABLE_SFML
#   include <SFML/Window.hpp>
#endif
#if GAP311_ENABLE_WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif

namespace GAP311
{
	/// This structure represents a light abstraction of the major components of a
	///	vk::GraphicsPipelineCreateInfo structure.
	/// 
	struct GraphicsPipelineConfig
	{
		///
		/// VERTEX INPUTS
		///
		/// Input Bindings and Input Attributes define aspects of what are referred to as
		///	per-vertex inputs. In the typical sense vertex inputs define how we send a polygonal
		///	mesh to the GPU for rendering. The GPU will act on each of these inputs, often in
		///	a massively-parallel environment. But how we store the data in system memory might
		///	differ from how the GPU would like it, so we use these descriptions to describe
		///	how the different components of a vertex (like position, normal, color, UV, etc.)
		///	are laid out in the vertex buffer(s) that will be used.
		///
		/// A Vertex Input Binding defines a slot (called a binding) that a vertex buffer can be
		///	plugged into during rendering. The binding defines the rate of input (per-vertex most commonly)
		///	and the stride between each vertex in the buffer. Think of this as defining aspects of a
		///	for loop if you were to loop over the buffer yourself. There will always be at least one binding.
		///
		std::vector<vk::VertexInputBindingDescription> vertexBindings = {};

		/// A Vertex Input Attribute defines a component of a vertex -- what binding it is in, what byte
		///	offset to apply to get to the data, and the format of the data itself.
		/// A simple pattern for defining a vertex in system memory might be to make a struct that represents
		///	each vertex and have a member variable for each component of the vertex. This would then correlate
		///	to a single binding that has a stride equal to the size of the struct and a vertex attribute for
		///	each member variable, using its offset and type.
		///
		std::vector<vk::VertexInputAttributeDescription> vertexAttributes = {};

		/// The last part of defining vertex inputs is to indicate how the vertexes correllate with
		///	each other to form a geometric primitive. Most commonly these are triangles of some form
		///	either in a list or a strip, the latter being a way to define adjacent triangles in a compact fashion.
		/// 
		vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;

		///
		/// UNIFORM INPUTS
		///
		/// Uniform (also known as constant) inputs are inputs to shaders that do not change
		/// during a draw call. So these values are "uniformly constant" across all shader program
		///	invocations in the drawing operation. Contrast this to vertex or stream inputs, which
		///	change for each invocation of a shader.
		///
		/// Push constants and descriptor sets are two ways of providing uniformly constant data.

		/// Push constants are constant values that are inserted directly into the command buffer
		///	using the vk::CommandBuffer::pushConstants function prior to the draw command.
		///	This is a mechanism for providing more granular constant data than a normal uniform buffer.
		///	An example application is to pass in the object's transform matrix via a push constant block.
		///
		std::vector<vk::PushConstantRange> shaderPushConstants = {};

		/// Descriptor sets are groupings of constant data provided to shaders. The provided data can be
		///	raw buffers, images, or other more complex types.
		///	Layouts are declarations -- mostly the size and format of the data. So providing the layouts
		///	here says that when we bind the pipeline for usage, we expect descriptor sets (the actual data)
		///	to be bound as well, filling in these layout "slots"
		///	
		std::vector<vk::DescriptorSetLayout> shaderUniforms = {};

		///
		/// SHADER PROGRAMS
		///
		/// This is where the code for each shader stage gets supplied. Only the vertex and fragment (pixel) stages
		///	are typically required, but other stages can be added as needed.
		///	The order specified here does not matter.
		///
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {};

		///
		/// RASTERIZATION
		///
		/// Rasterization state dictates how we take the geometric primitives and map them to color fragments (pixels)
		///	that will be passed to the fragment shader.
		///
		/// Polygon mode dictates if we're filling our polygon, drawing only the edges, or even just the vertex points themselves.
		///
		vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

		/// Cull mode and front face designation work together to perform an operation common to optimization
		///	and other rendering techniques: choosing whether or not to draw faces pointing away from the viewport.
		///
		vk::CullModeFlags cullMode = vk::CullModeFlagBits::eNone;
		///	Front face describes the winding -- order of vertexes -- that defines if the triangle is facing forward or backwards.
		///	Cull mode controls which directions will be skipped when rasterizing.
		///
		vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

		///
		/// DEPTH AND STENCIL
		///
		/// The depth and stencil state controls how the depth and stencil buffer operations are performed.
		///	Not all render passes have depth/stencil buffers, so these operations are only valid if a depth/stencil buffer is used.
		///
		///	Depth testing is used as a way to skip fragment (pixel) options by performing depth-based comparisons for the fragments
		///	that are generated. This can end up as an optimization (not processing a color fragment that is occluded by another) or
		///	as part of a rendering technique (checking against a light's depth map for shadow mapping)
		///
		///	Tests are only performed if enabled and a fragment is considered to pass the depth test if the comparison operation between
		///	its depth and the existing depth value at the same location evaluates to true.
		///
		vk::Bool32 depthTestEnable = false;
		vk::CompareOp depthCompareOp = vk::CompareOp::eNever;

		/// For some techniques (like drawing semitransparent objects) you might want to not write the value that passes the test back
		///	to the depth buffer, so that can be controlled separately.
		///
		vk::Bool32 depthWriteEnable = false;
		vk::Bool32 depthBiasEnable = false;

		///
		/// BLENDING
		///
		/// Once we have shaded a color fragment (or pixel) we must now incorporate it into the framebuffer that we are drawing into.
		/// A sensible default (provided below) just writes out the color value directly. But this is also where we can configure
		///	how semitransparent objects can be blended with the scene through a technique known as alpha blending.
		///
		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendStates =
		{
			// By default allow writing of all components
			vk::PipelineColorBlendAttachmentState()
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
		};

		///
		/// OUTPUT
		///
		/// The output stage defines where and into what you are drawing. This is where the results of the graphics pipeline end up.
		///
		/// As part of the output stages, the regions of the framebuffer that are being drawn into need to be defined.
		///	Most of the time you will have one viewport and one scissor
		///
		///	A viewport defines the region, in pixel coordinates, that is being rendered into. When drawing to the screen
		///	this is most commonly the screen resolution. Rendering will be scaled to fit in the viewport.
		///
		std::vector<vk::Viewport> viewports = {};

		/// A scissor rectangle defines a further constraint within the viewport.
		///	Most commonly the scissor dimensions will match the viewport.
		///  
		std::vector<vk::Rect2D> scissors = {};

		/// Finally, a render pass is an object that represents the formats of the output data being generated and into which
		///	color attachments of the framebuffer the data should be written into.
		///
		/// NOTE: This can be left null if using with VulkanWrapper::CreatePipeline()
		///
		vk::RenderPass renderPass;

		///
		/// DYNAMIC STATE
		///
		/// For optimization reasons, Vulkan prefers knowing as much about the intended state
		/// of the pipeline ahead of time. But this isn't always desired or sometimes possible,
		///	so there is a mechanism called Dynamic State which allows us to set parts of the pipeline
		///	state directly in the command buffer before we issue the draw call.
		///	To use this, we tell Vulkan what states it should expect to be set later by providing it
		///	an array of state identifiers.
		///
		std::vector<vk::DynamicState> dynamicStates = {};
	};

	///
	/// SHADER UNIFORMS
	///
	/// Uniforms are values that remain uniform across the entire duration of a draw call or similar execution on the GPU.
	///	Contrast these with the per-vertex attributes and per-instance attributes that are provided as vertex buffers.
	///	In Vulkan, these are called DescriptorSets. And the format (or type) of a DescriptorSet is a DescriptorSetLayout.
	///	The term Set is used here because there are often multiple resources that are grouped together and used in shaders.
	///	Though for most of our examples we will be creating DescriptorSets with only one Resource Descriptor in the set.
	///	
	///	And that's why the term Descriptor is used, these objects do not contain the data themselves but merely a reference
	///	to it. To compare it to C++, a DescriptorSetLayout is like a `class` or `struct` declaration -- what the members are.
	///	And consider each member being a pointer or reference. But until we instantiate it, there is no actual data.
	///	A DescriptorSet is an instantiation of a DescriptorSetLayout. And to associate resources (like buffers or images)
	///	with a DescriptorSet we write into it, similar to assigning a value to a pointer member.
	///
	///	Technically a Shader Uniform (also called a Shader Constant) is only one of a handful of additional resources
	///	that can be provided to a shader through the DescriptorSet architecture.
	struct ShaderUniform : std::tuple<vk::DescriptorSet, vk::DescriptorSetLayout, vk::DescriptorType, uint32_t>
	{
		vk::DescriptorSet       GetSet()     const { return std::get<0>(*this); }
		vk::DescriptorSetLayout GetLayout()  const { return std::get<1>(*this); }
		vk::DescriptorType      GetType()    const { return std::get<2>(*this); }
		uint32_t                GetBinding() const { return std::get<3>(*this); }

		operator vk::DescriptorSet()       const { return GetSet(); }
		operator vk::DescriptorSetLayout() const { return GetLayout(); }
	};

	/// 
	/// RENDER TARGETS
	/// 
	/// A render target (also referred to as a render texture, or offscreen render) is a collection of
	///	Vulkan resources set up to enable rendering to a framebuffer that is not one of the screen buffers.
	///	The backing image that is used in this rendering is often used in other rendering operations.
	///	For example, rendering a scene to a texture and then mapping that texture on an object in the world
	///	such as a TV or similar. But this is also useful for certain techniques that require rendering the
	///	world from a certain perspective, such as rendering a shadow map from the perspective of a light source,
	///	where the map is later used in the main render pass.
	///
	struct RenderTarget
	{
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> views;
		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;
	};

	///
	/// GRAPHICS PIPELINES
	///
	/// Pipelines are objects that encode the entire state of the execution pipeline on the GPU.
	///	This includes the shaders used, the format of vertex inputs, the format of any uniforms (extra resources),
	/// the rasterization settings, output formats, and more.
	///	This makes it harder to change part of the pipeline state dynamically, but improves performance
	///	significantly, which is why all major APIs have adopted this kind of architecture.
	///
	///	In Vulkan there are two objects that are often used together: vk::Pipeline and vk::PipelineLayout.
	///	The Pipeline object contains all the state information about the pipeline, and it even uses PipelineLayout.
	///	The PipelineLayout object only contains information about non-vertex resources needed by the shaders. This
	///	usually means the uniforms exposed to shaders. Not all Pipelines need PipelineLayout objects, especially
	///	if they aren't taking in resources like uniforms.
	///	The GraphicsPipeline object we create here wraps them both for ease of use.
	///
	///	One of the more annoying aspects of creating a Pipeline is that it needs to know the RenderPass
	///	that it will be used within. When drawing to the backbuffer/swapchain (which is what a lot of our drawing
	///	operations target) this means we have to recreate the pipeline whenever the swapchain's RenderPass
	///	changes. Which could be due to switching resolutions or going fullscreen. To reduce the amount of code
	///	needed to manage that, the CreatePipeline helper will return a shared object and the wrapper holds a
	///	weak list of pipelines that it will automatically swap out the RenderPass and recreate the Pipeline
	///	as needed. This is also why it stores a copy of the configuration object.
	///
	struct GraphicsPipeline : std::tuple<vk::Pipeline, vk::PipelineLayout, GraphicsPipelineConfig>
	{
		vk::Pipeline                  GetPipeline() const { return std::get<0>(*this); }
		vk::PipelineLayout            GetLayout()   const { return std::get<1>(*this); }
		const GraphicsPipelineConfig& GetConfig()   const { return std::get<2>(*this); }

		operator vk::Pipeline()       const { return GetPipeline(); }
		operator vk::PipelineLayout() const { return GetLayout(); }
	};

	/// This class provides a simpler abstraction of parts of the Vulkan API.
	///	It does not intend to be feature-complete and 100% bulletproof. This will
	///	capture the basic usage and interaction but you are still free to write raw
	///	Vulkan calls on your own.
	///
	class VulkanWrapper
	{
	public:
		using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;
		using ConfigureInstanceFunc = std::function<void(vkb::InstanceBuilder&)>;
		using ConfigureDeviceFunc = std::function<void(vkb::PhysicalDeviceSelector&)>;
		using DebugLogFunc = std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*)>;

		static inline DebugLogFunc VulkanLogToSTDOUT(bool breakOnError = true);

		///
		/// FRAMEWORK INTEGRATION HELPERS
		///
		/// These are integration helper methods allowing the Vulkan utilities
		///	to interact with frameworks such as SDL, GLFW, SFML, and more.
		///	These mostly are for integration with the windowing capabilities of each
		///	but on some platforms there is additional support for other utilities
		///	like logging and format conversion.
		///
#if GAP311_ENABLE_SDL
		static inline CreateSurfaceFunc CreateVulkanSurfaceSDL(SDL_Window* pWindow);
		static inline DebugLogFunc VulkanLogToSDL(bool breakOnError = true);
		static inline vk::Format VulkanFormatFromSDLFormat(Uint32 sdl_format);
#endif
#if GAP311_ENABLE_GLFW
		static inline CreateSurfaceFunc CreateVulkanSurfaceGLFW(GLFWwindow* pWindow);
#endif
#if GAP311_ENABLE_SFML
		static inline CreateSurfaceFunc CreateVulkanSurfaceSFML(sf::Window* pWindow);
		static inline DebugLogFunc VulkanLogToSfErr(bool breakOnError = true);
#endif
#if GAP311_ENABLE_WIN32
		static inline CreateSurfaceFunc CreateVulkanSurfaceWin32(HWND hWindow);
		static inline DebugLogFunc VulkanLogToWin32DebugString(bool breakOnError = true);
#endif

		///
		/// CONFIGURATION AND LIFECYCLE
		///

		struct ConfigOptions
		{
			// For simplification the wrapper turns off depth/stencil support by default.
			bool enableDepthStencilBuffer = false;

			// To refine the Vulkan Device or Instance creation process, provide callbacks here
			ConfigureDeviceFunc configureDeviceFunc;
			ConfigureInstanceFunc configureInstanceFunc;
			// To enable debugging and validation, provide a logging callback
			// A framework-specific helper might be available for this, see the VulkanLogTo* methods
			DebugLogFunc debugLogFunc;

			// Vulkan requires upper limits to be defined when it comes to both the
			// maximum number of descriptor sets (values plugged into a shader) as well as
			// the types of those sets. These are some reasonable defaults for the narrow
			// scope of the course.
			std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
			{
				{ vk::DescriptorType::eUniformBuffer, 128 },
				{ vk::DescriptorType::eCombinedImageSampler, 128 },
			};
			uint32_t maxDescriptorSets = 256;
		};

		bool Initialize(const char* pApplicationName,  // Vulkan requires an application name string
			CreateSurfaceFunc&& createSurfaceFunc,     // A function to create a vk::SurfaceKHR object is required
			const ConfigOptions& options = {}          // Optional settings
		);

		void Shutdown();

		///
		/// Helpers to retrieve Vulkan C++ wrapper classes for various core objects.
		///

		[[nodiscard]] vk::Instance        GetInstance()             const { return m_vkbInstance.instance; }
		[[nodiscard]] vk::Device          GetDevice()               const { return m_vkbDevice.device; }
		[[nodiscard]] vk::PhysicalDevice  GetPhysicalDevice()       const { return m_vkbPhysicalDevice.physical_device; }
		[[nodiscard]] vkb::Swapchain      GetSwapchain()            const { return m_vkbSwapchain; }
		[[nodiscard]] vk::SurfaceKHR      GetDisplaySurface()       const { return m_displaySurface; }
		[[nodiscard]] vk::Viewport        GetDisplayViewport()      const { return m_displayViewport; }
		[[nodiscard]] vk::RenderPass      GetDisplayRenderPass()    const { return m_displayRenderPass; }
		[[nodiscard]] vk::Queue           GetGraphicsQueue()        const { return m_graphicsQueue; }
		[[nodiscard]] uint32_t            GetGraphicsQueueIndex()   const { return m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value(); }
		[[nodiscard]] vk::CommandPool     GetGraphicsCommandPool()  const { return m_graphicsCommandPool; }
		[[nodiscard]] vk::Queue           GetPresentQueue()         const { return m_presentQueue; }
		[[nodiscard]] vk::DescriptorPool  GetDescriptorPool()       const { return m_descriptorPool; }
		[[nodiscard]] vk::PipelineCache   GetPipelineCache()        const { return m_pipelineCache; }

		///
		/// COMMAND EXECUTION
		///
		/// These methods provide means of queueing and executing commands that are stored in a command buffer.
		///

		/// These methods bookend rendering commands intended for the screen
		///	A command buffer and framebuffer for the next rendering "frame" are returned
		///	and these operations will be submitted when EndFrame is called.
		bool BeginFrame(vk::CommandBuffer& commandBuffer, vk::Framebuffer& framebuffer);
		void EndFrame();

		/// For times when you need to execute commands on the GPU that do not result in
		///	graphical output, like uploading data when creating a resource, this method
		///	accepts a function that will be provided a command buffer to write commands into
		///	which will be executed immediately after the function returns.
		///	This method will wait for the commands to finish executing before returning.
		bool ExecuteCommands(std::function<void(vk::CommandBuffer&)>&& generateCommands);

		/// 
		/// BUFFER RESOURCES
		///
		/// Buffers are effectively arrays or blobs of memory for usage by the GPU.
		///	Vulkan separates memory allocation from resource creation and so these methods will store
		///	the memory object so it does not need to be directly managed. Because of this, the DestroyBuffer
		///	method must be used with CreateBuffer.
		///

		/// Create a buffer given a full info struct, optionally copying initial data to it.
		///	The info struct must be configured properly if initial data is provided and the initial data must
		///	be the exact size of the buffer. This function will not perform any saftey checks.
		///	See the other helpers for safer versions.
		[[nodiscard]] vk::Buffer CreateBuffer(const vk::BufferCreateInfo& createInfo, const void* pInitialData = nullptr);

		/// Create a buffer given its usage and size, optionally copying initial data to it.
		///	This method will create the buffer with TransferDst usage if initial data is supplied.
		[[nodiscard]] vk::Buffer CreateBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, const void* pInitialData = nullptr);

		/// Simple creation helpers for common usages
		[[nodiscard]] vk::Buffer CreateIndexBuffer(vk::DeviceSize size, const void* pInitialData = nullptr)   { return CreateBuffer(vk::BufferUsageFlagBits::eIndexBuffer, size, pInitialData); }
		[[nodiscard]] vk::Buffer CreateVertexBuffer(vk::DeviceSize size, const void* pInitialData = nullptr)  { return CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer, size, pInitialData); }
		[[nodiscard]] vk::Buffer CreateUniformBuffer(vk::DeviceSize size, const void* pInitialData = nullptr) { return CreateBuffer(vk::BufferUsageFlagBits::eUniformBuffer, size, pInitialData); }

		/// Destroy a buffer created with a CreateBuffer method.
		///	This will lookup the associated memory object and free it.
		void DestroyBuffer(vk::Buffer buffer);

		/// Lookup the memory object associated with a buffer
		[[nodiscard]] vk::DeviceMemory GetBufferMemoryHandle(vk::Buffer buffer) const;

		///
		/// IMAGE RESOURCES
		///
		/// Images are very similar to buffers in that they are blobs or arrays of memory.
		///	A major difference is that an image knows what type of "pixel" data it contains, so it can be stored
		///	and processed more efficiently by the GPU. Images can also have 1 to 3 dimensions and employ features
		///	such as mipmaps. Because Vulkan separates memory allocation from resource creation, any image created
		///	with CreateImage should be destroyed with DestroyImage so the associated memory is freed.
		///
		/// Unlike buffers, images generally are not used directly but instead through ImageView objects that
		///	specify how to interpret the underlying image data. So some helpers also provide facilities to
		///	simultaneously create an ImageView object in addition to the Image.
		///

		/// Create an image given its info structure and optionally some initial data to upload to it.
		///	This method does not provide any checks to ensure the info struct was set up correctly.
		///	Use one of the other helper methods for better safety.
		[[nodiscard]] vk::Image CreateImage(const vk::ImageCreateInfo& imageInfo, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);

		/// Create an image given its info structure and create a view with a configuration derived from the image.
		[[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateImageAndView(const vk::ImageCreateInfo& imageInfo, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);

		/// Create an Image and ImageView for use as a 2D sampled texture.
		[[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateTexture2DImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);

		/// Create a 6-layered image to be used as a cubemap. The initial data has each image for each face, one after the other.
		[[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateCubemapImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);

		/// Destroy an image, also freeing its associated memory object
		void DestroyImage(vk::Image image);
		/// Helper to also destroy an image view at the same time
		void DestroyImageAndView(vk::Image image, vk::ImageView view);

		/// Lookup the memory object associated with the image
		[[nodiscard]] vk::DeviceMemory GetImageMemoryHandle(vk::Image image) const;

		/// Helper to upload data to an image using a staging buffer
		bool UploadImageData(vk::Image image, vk::Extent3D extents, const void* pData, vk::DeviceSize dataSize, uint32_t layerCount = 1);

		/// Create a GraphicsPipeline object. The RenderPass will automatically be set to the display's RenderPass
		///	when it is available if the provided RenderPass is null or matches the current display's RenderPass.
		///	See GraphicsPipelineConfig for more details.
		[[nodiscard]] std::shared_ptr<GraphicsPipeline> CreatePipeline(const GraphicsPipelineConfig& config);

		/// Destroy the managed GraphicsPipeline object and null out the pointer.
		void DestroyPipeline(std::shared_ptr<GraphicsPipeline>& pipeline);

		/// This is a lightweight helper to create the raw Vulkan handles instead of a managed GraphicsPipeline object.
		[[nodiscard]] std::tuple<vk::Pipeline, vk::PipelineLayout> CreatePipelineAndLayout(const GraphicsPipelineConfig& config);

		/// Helper to create a DescriptorSet pairing for use with a constant buffer
		[[nodiscard]] ShaderUniform CreateUniformForBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);

		/// Helper to create a DescriptorSet pairing for use with an image for sampling
		[[nodiscard]] ShaderUniform CreateUniformForImage(uint32_t binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);

		/// Destroy the shader uniform resources
		void DestroyUniform(ShaderUniform uniform);

		/// ShaderUniforms by default contain both a layout and a set that satisfies the layout
		///	But when dealing with images, it is cumbersome to update the set to point to a new image
		///	This helper will create a new DescriptorSet which can be provided to bindDescriptorSets instead
		///	of the original ShaderUniform.
		vk::DescriptorSet CreateAlternateImageValueForUniform(ShaderUniform uniform, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

		/// When creating a new DescriptorSet, use this method to handle freeing it when you're done
		void FreeDescriptorSet(vk::DescriptorSet set);

		///
		/// RENDER PASSES
		///
		/// In a conventional sense, a Render Pass is when a series of draw calls are issued on a set of outputs.
		///	Multi-pass rendering usually involves drawing the same objects multiple times but with different outputs and pipeline settings.
		///	Often, the output of one pass is fed into another pass to perform certain rendering techniques.
		///
		///	The RenderPass object, when it comes to Vulkan, is just information about the output from a pass.
		/// These helpers will create some of the common output formats.
		///	When it comes to ImageLayouts, these are usage instructions to the GPU. There are different ways to store (or access)
		///	image data depending on what operation is being performed. Because performance is key, this is something we now need to explicitly specify.
		///

		/// Create a RenderPass that has a single color output of the given format. Layout defaults to being presentable, but you may want to use
		///	eShaderReadOnlyOptimal if drawing to a texture that will later be used in a shader.
		[[nodiscard]] vk::RenderPass CreateColorOnlyRenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);
		/// Creates both a Color and DepthStencil capable RenderPass.
		[[nodiscard]] vk::RenderPass CreateColorAndDepthRenderPass(vk::Format colorFormat, vk::Format depthFormat, vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);

		/// Creates a collection of resources to use as a render target, given the size and formats of the color and/or depth values.
		///	At least one format needs to be specified as images will not be created for eUndefined.
		[[nodiscard]] RenderTarget CreateRenderTarget(vk::Extent2D extents, vk::Format colorFormat = vk::Format::eUndefined, vk::Format depthFormat = vk::Format::eUndefined);

		/// Destroy all the created resources for the render target
		void DestroyRenderTarget(RenderTarget renderTarget);

		///
		/// UTILITIES
		///

		/// Resolve memory requirements into the type of memory that should be allocated
		[[nodiscard]] int FindMemoryTypeIndex(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags);

		/// Allocate device memory that fulfills the provided requirements
		[[nodiscard]] vk::DeviceMemory AllocateMemory(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags);

	private:
		/// There are a number of reasons a swapchain can become invalidated (like resizing the window)
		///	so we organize the recreation into a single method.
		bool RebuildSwapchain();

		static VkBool32 DebugLogCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		DebugLogFunc m_debugLogFunc;

		std::unordered_map<VkBuffer, vk::DeviceMemory> m_bufferMemoryMap;
		std::unordered_map<VkImage, vk::DeviceMemory> m_imageMemoryMap;
		std::vector<std::weak_ptr<GraphicsPipeline>> m_graphicsPipelines;

		vkb::Instance       m_vkbInstance;
		vkb::Device         m_vkbDevice;
		vkb::PhysicalDevice m_vkbPhysicalDevice;
		vkb::Swapchain      m_vkbSwapchain;

		vk::SurfaceKHR      m_displaySurface;
		vk::Viewport        m_displayViewport;
		vk::RenderPass      m_displayRenderPass;

		vk::Format          m_depthFormat;
		vk::ImageView       m_depthStencilView;
		vk::Image           m_depthStencilImage;
		
		vk::CommandPool     m_graphicsCommandPool;
		vk::DescriptorPool  m_descriptorPool;
		vk::PipelineCache   m_pipelineCache;

		vk::Queue           m_graphicsQueue;
		vk::Queue           m_presentQueue;

		struct FramebufferData
		{
			vk::Framebuffer framebuffer;
			vk::ImageView imageView;
			vk::CommandBuffer commandBuffer;
			vk::Fence inUse;
		};
		std::vector<FramebufferData> m_framebuffers;
		uint32_t m_currentFramebufferIndex = 0;

		static constexpr uint32_t kMaxPendingFrames = 2;
		struct FrameSyncStatus
		{
			vk::Semaphore isImageAvailable;
			vk::Semaphore isRenderFinished;
			vk::Fence inUse;
		};
		FrameSyncStatus m_frames[kMaxPendingFrames];
		uint32_t m_currentFrameIndex = 0;
	};

	////////////////// Implementation //////////////////

	constexpr bool ShouldBreakOnError(bool enable, VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type)
	{
		// NOTE: Some layers can throw "errors" but we don't actually care about them
		// so we limit breaking on validation errors only.
		return enable
			&& ::IsDebuggerPresent()
			&& severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
			&& (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT);
	}

	inline VulkanWrapper::DebugLogFunc VulkanWrapper::VulkanLogToSTDOUT(bool breakOnError)
	{
		return [breakOnError](
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* pData)
			{
				// vkBootstrap's default goes to STDOUT
				vkb::default_debug_callback(severity, type, pData, nullptr);
				if (ShouldBreakOnError(breakOnError, severity, type))
					__debugbreak();
			};
	}

#if GAP311_ENABLE_SDL
	inline VulkanWrapper::CreateSurfaceFunc VulkanWrapper::CreateVulkanSurfaceSDL(SDL_Window* pWindow)
	{
		return [pWindow](vk::Instance instance) -> vk::SurfaceKHR
		{
			VkSurfaceKHR surface;
			SDL_Vulkan_CreateSurface(pWindow, instance, &surface);
			return surface;
		};
	}
	inline VulkanWrapper::DebugLogFunc VulkanWrapper::VulkanLogToSDL(bool breakOnError)
	{
		return [breakOnError](
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,	
			const VkDebugUtilsMessengerCallbackDataEXT* pData)
			{
				auto ms = vkb::to_string_message_severity(severity);
				auto mt = vkb::to_string_message_type(type);
				SDL_Log("[Vulkan][%s %s] %s", ms, mt, pData->pMessage);
				if (ShouldBreakOnError(breakOnError, severity, type))
	 					SDL_TriggerBreakpoint();
			};
	}		
	inline vk::Format VulkanWrapper::VulkanFormatFromSDLFormat(Uint32 sdl_format)
	{
		switch (sdl_format)
		{
		// This is not a complete mapping
		// SDL enums list components in reverse order

		// 4-component 8-bit element
		case SDL_PIXELFORMAT_ABGR8888: return vk::Format::eR8G8B8A8Unorm;
		case SDL_PIXELFORMAT_ARGB8888: return vk::Format::eB8G8R8A8Unorm;
		case SDL_PIXELFORMAT_RGBA8888: return vk::Format::eA8B8G8R8UnormPack32;
		// 3-component 8-bit element
		case SDL_PIXELFORMAT_RGB888:   return vk::Format::eB8G8R8Unorm;
		case SDL_PIXELFORMAT_BGR888:   return vk::Format::eR8G8B8Unorm;
		case SDL_PIXELFORMAT_UNKNOWN:
		default:
			break;
		}
		return vk::Format::eUndefined;
	}
#endif
#if GAP311_ENABLE_GLFW
	inline VulkanWrapper::CreateSurfaceFunc VulkanWrapper::CreateVulkanSurfaceGLFW(GLFWwindow* pWindow)
	{
		return [pWindow](vk::Instance instance) -> vk::SurfaceKHR
		{
			VkSurfaceKHR surface;
			glfwCreateWindowSurface(instance, pWindow, nullptr, &surface);
			return surface;
		};
	}
#endif
#if GAP311_ENABLE_SFML
	inline VulkanWrapper::CreateSurfaceFunc VulkanWrapper::CreateVulkanSurfaceSFML(sf::Window* pWindow)
	{
		return [pWindow](vk::Instance instance) -> vk::SurfaceKHR
		{
			vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
			surfaceInfo.hInstance = ::GetModuleHandle(NULL);
			surfaceInfo.hwnd = reinterpret_cast<HWND>(pWindow->getSystemHandle());
			return instance.createWin32SurfaceKHR(surfaceInfo);
		};
	}
	inline VulkanWrapper::DebugLogFunc VulkanWrapper::VulkanLogToSfErr(bool breakOnError)
	{
		return [breakOnError](
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* pData)
			{
				auto ms = vkb::to_string_message_severity(severity);
				auto mt = vkb::to_string_message_type(type);
				sf::err() << "[Vulkan][" << ms << " " << mt << "] " << pData->pMessage;
				if (ShouldBreakOnError(breakOnError, severity, type))
					__debugbreak();
			};
	}
#endif
#if GAP311_ENABLE_WIN32
	inline VulkanWrapper::CreateSurfaceFunc VulkanWrapper::CreateVulkanSurfaceWin32(HWND hWindow)
	{
		return [hWindow](vk::Instance instance) -> vk::SurfaceKHR
		{
			vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
			surfaceInfo.hInstance = ::GetModuleHandle(NULL);
			surfaceInfo.hwnd = hWindow;
			return instance.createWin32SurfaceKHR(surfaceInfo);
		};
	}
	inline VulkanWrapper::DebugLogFunc VulkanWrapper::VulkanLogToWin32DebugString(bool breakOnError)
	{
		return [breakOnError](
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* pData)
			{
				auto ms = vkb::to_string_message_severity(severity);
				auto mt = vkb::to_string_message_type(type);
				::OutputDebugStringA("[Vulkan][");
				::OutputDebugStringA(ms);
				::OutputDebugStringA(" ");
				::OutputDebugStringA(mt);
				::OutputDebugStringA("] ");
				::OutputDebugStringA(pData->pMessage);
				::OutputDebugStringA("\n");
				if (ShouldBreakOnError(breakOnError, severity, type))
					::DebugBreak();
			};
	}
#endif

	inline bool VulkanWrapper::Initialize(const char* pApplicationName, CreateSurfaceFunc&& createSurfaceFunc,
		const ConfigOptions& options)
	{
		if (!createSurfaceFunc)
			return false;

		// Vulkan must first have an instance created, which will act as the
		// DLL context for the app
		auto instBuilder = vkb::InstanceBuilder()
			.set_app_name(pApplicationName);

		if (options.debugLogFunc)
		{
			m_debugLogFunc = options.debugLogFunc;
			instBuilder.set_debug_callback(DebugLogCallback);
			instBuilder.set_debug_callback_user_data_pointer(this);

			// If a user is providing a logging function, assume they want validation checks
			instBuilder.request_validation_layers();
		}
		
		if (options.configureInstanceFunc)
			options.configureInstanceFunc(instBuilder);

		auto instResult = instBuilder.build();
		if (!instResult)
			return false;

		m_vkbInstance = instResult.value();

		// Since we're going to be outputting graphics to a window, we need a
		// surface for vulkan to render into

		m_displaySurface = createSurfaceFunc(m_vkbInstance.instance);
		if (!m_displaySurface)
			return false;

		// Now that we have a surface we can look for the physical device which can draw to this surface

		auto selector = vkb::PhysicalDeviceSelector(m_vkbInstance)
			.set_surface(m_displaySurface);

		if (options.configureDeviceFunc)
			options.configureDeviceFunc(selector);

		auto selectResult = selector.select();
		if (!selectResult)
			return false;

		m_vkbPhysicalDevice = selectResult.value();

		// Finally, we can create a logical device using the physical device, all our commands will go through the logical device

		auto deviceResult = vkb::DeviceBuilder(m_vkbPhysicalDevice).build();
		if (!deviceResult)
			return false;

		m_vkbDevice = deviceResult.value();

		m_graphicsQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics).value();
		if (!m_graphicsQueue)
			return false;

		m_presentQueue = m_vkbDevice.get_queue(vkb::QueueType::present).value();
		if (!m_presentQueue)
			return false;

		vk::Device device(m_vkbDevice.device);

		// Now with basic device setup out of the way we need to finish creating the objects
		// that will allow us to issue rendering commands to the window that will be displayed

		// One command pool will house all the graphics commands we issue
		m_graphicsCommandPool = device.createCommandPool(
			vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value())
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		);
		if (!m_graphicsCommandPool)
			return false;

		// A descriptor pool holds all the types of descriptor sets (shader uniforms) we will be allocating
		// to set shader values.
		m_descriptorPool = device.createDescriptorPool(
			vk::DescriptorPoolCreateInfo()
				.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
				.setMaxSets(options.maxDescriptorSets)
				.setPoolSizes(options.descriptorPoolSizes)
		);
		if (!m_descriptorPool)
			return false;

		// A pipeline cache is not required but can speed up the creation of duplicate pipelines
		m_pipelineCache = device.createPipelineCache(
			vk::PipelineCacheCreateInfo()
		);

		// Create the synchronization primitives used to organize our multi-buffered rendering
		for (auto& frame : m_frames)
		{
			frame.isImageAvailable = device.createSemaphore(vk::SemaphoreCreateInfo());
			frame.isRenderFinished = device.createSemaphore(vk::SemaphoreCreateInfo());
			frame.inUse = device.createFence(
				vk::FenceCreateInfo()
					.setFlags(vk::FenceCreateFlagBits::eSignaled)
			);
		}
		m_currentFrameIndex = 0;

		// Normally we would query the physical device to find out a good depth/stencil format
		// that matches what we want, but 24-bits of depth and 8-bits of stencil information
		// is widely supported / fairly standardized so it should be supported on all gpus.
		if (options.enableDepthStencilBuffer)
			m_depthFormat = vk::Format::eD24UnormS8Uint;
		else
			m_depthFormat = vk::Format::eUndefined;

		// Build the swapchain to set up our output framebuffers and any related resources like render passes
		if (!RebuildSwapchain())
			return false;

		return true;
	}

	inline void VulkanWrapper::Shutdown()
	{
		if (const vk::Device device = m_vkbDevice.device)
		{
			device.waitIdle();

			m_graphicsPipelines.clear();

			if (m_displayRenderPass)
			{
				device.destroyRenderPass(m_displayRenderPass);
				m_displayRenderPass = vk::RenderPass();
			}

			std::vector<vk::CommandBuffer> commandBuffers;
			commandBuffers.reserve(m_framebuffers.size());
			std::vector<VkImageView> imageViews;
			imageViews.reserve(m_framebuffers.size());
			for (const auto& fbData : m_framebuffers)
			{
				if (fbData.imageView)
					imageViews.push_back(fbData.imageView);
				if (fbData.framebuffer)
					device.destroyFramebuffer(fbData.framebuffer);
				if (fbData.commandBuffer)
					commandBuffers.push_back(fbData.commandBuffer);
			}
			m_framebuffers.clear();
			m_currentFramebufferIndex = 0;

			if (!commandBuffers.empty())
				device.freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
			if (!imageViews.empty())
				m_vkbSwapchain.destroy_image_views(imageViews);

			if (m_depthStencilImage)
			{
				DestroyImageAndView(m_depthStencilImage, m_depthStencilView);
				m_depthStencilImage = vk::Image();
				m_depthStencilView = vk::ImageView();
			}

			for (auto& frame : m_frames)
			{
				if (frame.isImageAvailable)
				{
					device.destroySemaphore(frame.isImageAvailable);
					frame.isImageAvailable = vk::Semaphore();
				}

				if (frame.isRenderFinished)
				{
					device.destroySemaphore(frame.isRenderFinished);
					frame.isRenderFinished = vk::Semaphore();
				}

				if (frame.inUse)
				{
					device.destroyFence(frame.inUse);
					frame.inUse = vk::Fence();
				}
			}
			m_currentFrameIndex = 0;

			if (m_graphicsCommandPool)
			{
				device.destroyCommandPool(m_graphicsCommandPool);
				m_graphicsCommandPool = vk::CommandPool();
			}

			if (m_descriptorPool)
			{
				device.destroyDescriptorPool(m_descriptorPool);
				m_descriptorPool = vk::DescriptorPool();
			}

			if (m_pipelineCache)
			{
				device.destroyPipelineCache(m_pipelineCache);
				m_pipelineCache = vk::PipelineCache();
			}
		}

		vkb::destroy_swapchain(m_vkbSwapchain);
		vkb::destroy_device(m_vkbDevice);
		m_vkbPhysicalDevice = vkb::PhysicalDevice();

		if (const vk::Instance instance = m_vkbInstance.instance)
		{
			if (m_displaySurface)
			{
				instance.destroySurfaceKHR(m_displaySurface);
				m_displaySurface = vk::SurfaceKHR();
			}
		}

		vkb::destroy_instance(m_vkbInstance);
	}

	inline bool VulkanWrapper::RebuildSwapchain()
	{
		auto device = GetDevice();
		device.waitIdle();

		auto oldSwapchainImageFormat = vk::Format(m_vkbSwapchain.image_format);

		// Destroy the old per-frame resources as they will be recreated
		{
			std::vector<vk::CommandBuffer> commandBuffers;
			commandBuffers.reserve(m_framebuffers.size());
			std::vector<VkImageView> imageViews;
			imageViews.reserve(m_framebuffers.size());
			for (const auto& fbData : m_framebuffers)
			{
				if (fbData.imageView)
					imageViews.push_back(fbData.imageView);
				if (fbData.framebuffer)
					device.destroyFramebuffer(fbData.framebuffer);
				if (fbData.commandBuffer)
					commandBuffers.push_back(fbData.commandBuffer);
			}
			m_framebuffers.clear();
			m_currentFramebufferIndex = 0;

			if (!commandBuffers.empty())
				device.freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
			if (!imageViews.empty())
				m_vkbSwapchain.destroy_image_views(imageViews);
		}

		auto buildResult = vkb::SwapchainBuilder(m_vkbDevice)
			.set_old_swapchain(m_vkbSwapchain)
			.build();
		if (!buildResult)
			return false;
		
		m_vkbSwapchain = buildResult.value();

		m_displayViewport.width = m_vkbSwapchain.extent.width * 1.0f;
		m_displayViewport.height = m_vkbSwapchain.extent.height * 1.0f;
		m_displayViewport.minDepth = 0.0f;
		m_displayViewport.maxDepth = 1.0f;

		// recreate depth image as the swapchain size might have changed
		if (m_depthFormat != vk::Format::eUndefined)
		{
			if (m_depthStencilImage)
			{
				DestroyImageAndView(m_depthStencilImage, m_depthStencilView);
				m_depthStencilImage = vk::Image();
				m_depthStencilView = vk::ImageView();
			}

			std::tie(m_depthStencilImage, m_depthStencilView) = CreateImageAndView(
				vk::ImageCreateInfo()
					.setFormat(m_depthFormat)
					.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
					.setExtent(vk::Extent3D(m_vkbSwapchain.extent, 1))
					.setImageType(vk::ImageType::e2D)
					.setMipLevels(1)
					.setArrayLayers(1)
			);
		}

		// If the color format if our swapchain has changed, rebuild the associated render pass
		// We could just unconditionally rebuild, too, but since the format is unlikely to change
		// this will save having to rebuild any associated pipeline objects.
		if (!m_displayRenderPass || vk::Format(m_vkbSwapchain.image_format) != oldSwapchainImageFormat)
		{
			if (m_displayRenderPass)
			{
				// Destroy any pipeline objects that are referencing the old render pass

				std::unordered_set<vk::Pipeline> pendingDestroyPipelines;
				std::unordered_set<vk::PipelineLayout> pendingDestroyLayouts;

				erase_if(m_graphicsPipelines, [](const std::weak_ptr<GraphicsPipeline>& p) -> bool { return p.expired(); });
				for (auto& ptr : m_graphicsPipelines)
				{
					auto pipeline = ptr.lock();
					if (auto& renderPass = std::get<2>(*pipeline).renderPass;
						!renderPass || renderPass == m_displayRenderPass)
					{
						if (auto& pipelineRef = std::get<0>(*pipeline))
						{
							pendingDestroyPipelines.insert(pipelineRef);
							pipelineRef = vk::Pipeline();
						}
						if (auto& layout = std::get<1>(*pipeline))
						{
							pendingDestroyLayouts.insert(layout);
							layout = vk::PipelineLayout();
						}
						renderPass = vk::RenderPass();
					}
				}

				for (const auto& pipeline : pendingDestroyPipelines)
					device.destroyPipeline(pipeline);
				for (const auto& layout : pendingDestroyLayouts)
					device.destroyPipelineLayout(layout);
			}

			device.destroyRenderPass(m_displayRenderPass);
			m_displayRenderPass = m_depthFormat != vk::Format::eUndefined ?
				CreateColorAndDepthRenderPass(vk::Format(m_vkbSwapchain.image_format), m_depthFormat) :
				CreateColorOnlyRenderPass(vk::Format(m_vkbSwapchain.image_format));
			if (!m_displayRenderPass)
				return false;

			// Recreate any pipelines that output to the screen (swapchain) as they need the corresponding
			// RenderPass object
			erase_if(m_graphicsPipelines, [](const auto& p) -> bool { return p.expired(); });
			for (auto& ptr : m_graphicsPipelines)
			{
				auto pipeline = ptr.lock();
				if (auto& renderPass = std::get<2>(*pipeline).renderPass; !renderPass)
				{
					auto& pipelineRef = std::get<0>(*pipeline);
					auto& layout = std::get<1>(*pipeline);
					renderPass = m_displayRenderPass;
					std::tie(pipelineRef, layout) = CreatePipelineAndLayout(pipeline->GetConfig());
				}
			}
		}
	
		// Create the framebuffer objects associated with each image in our swapchain

		auto imageViews = m_vkbSwapchain.get_image_views().value();
		m_framebuffers.reserve(imageViews.size());

		auto frameCommandBuffers = device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_graphicsCommandPool)
				.setCommandBufferCount(static_cast<uint32_t>(imageViews.size()))
		);

		std::vector<vk::ImageView> imageAttachments = { vk::ImageView() };

		if (m_depthStencilView)
			imageAttachments.push_back(m_depthStencilView);

		for (vk::ImageView imageView : imageViews)
		{
			imageAttachments[0] = imageView;

			auto framebuffer = device.createFramebuffer(
				vk::FramebufferCreateInfo()
					.setAttachments(imageAttachments)
					.setRenderPass(m_displayRenderPass)
					.setWidth(m_vkbSwapchain.extent.width)
					.setHeight(m_vkbSwapchain.extent.height)
					.setLayers(1)
			);
			if (!framebuffer)
				return false;
			
			m_framebuffers.push_back({ framebuffer, imageView, frameCommandBuffers.back() });
			frameCommandBuffers.pop_back();
		}
		m_currentFramebufferIndex = 0;

		return true;
	}

	inline VkBool32 VulkanWrapper::DebugLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		auto self = static_cast<VulkanWrapper*>(pUserData);
		self->m_debugLogFunc(messageSeverity, messageTypes, pCallbackData);
		return VK_FALSE;
	}

	inline vk::RenderPass VulkanWrapper::CreateColorOnlyRenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout)
	{
		// An attachment describes an image that is written into by our rendering operations
		//	This is typically represented as an output from the fragment shader stage
		//	Like bindings, attachments are numerically referenced in the shader and
		//	if unspecified, attachment 0 is assumed.
		//	The layout here describes the data layout of the image that should be expected when
		//	all rendering is finished.
		auto colorAttachment = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// An attachment description describes more information about how color (or similar) data
		//	should be interpreted by the output and blending stage, so this is where the expected
		//	color format is defined as well as what to do when reading/writing (load/store) to the
		//	attachment. Final layout here describes the format the image should be transformed into
		//	when all output and blending operations are complete. For example, we want an attachment
		//	that is going to be displayed to the screen (the backbuffer/framebuffer) in ePresentSrcKHR.
		auto colorAttachmentDescription = vk::AttachmentDescription()
			.setFormat(colorFormat)
			.setFinalLayout(colorFinalLayout)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// A subpass defines what attachments are used when a multi-pass rendering operation is performed.
		//	But for our purposes we'll only have the required minimum of one subpass.
		auto subpassDesc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colorAttachment);

		// Each subpass should declare its dependencies for synchronization purposes.
		// The way this is done is by declaring what data access is performed between the particular stages
		//	that the subpass operates upon.
		//	Again, since we just have one subpass we use the special value of VK_SUBPASS_EXTERNAL.
		//	We're only operating on the attachments during the color output stages and need write access to
		//	write new color information but also read access to perform color blending.
		auto subpassDep = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		return GetDevice().createRenderPass(vk::RenderPassCreateInfo()
			.setAttachments(colorAttachmentDescription)
			.setSubpasses(subpassDesc)
			.setDependencies(subpassDep)
		);
	}

	inline vk::RenderPass VulkanWrapper::CreateColorAndDepthRenderPass(vk::Format colorFormat, vk::Format depthFormat, vk::ImageLayout colorFinalLayout)
	{
		// Set up a color attachment, see CreateColorRenderPass for more details
		auto colorAttachment = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
		auto colorAttachmentDescription = vk::AttachmentDescription()
			.setFormat(colorFormat)
			.setFinalLayout(colorFinalLayout)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// For depth, we set up another attachment but in the next slot
		// The difference here is that we will not be presenting this attachment to the screen
		//	so both the initial layout and final layout are the same.
		auto depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		auto depthStencilAttachmentDescription = vk::AttachmentDescription()
			.setFormat(depthFormat)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Subpass information is very similar to that of a typical color-only pass but we
		//	must also call out that the depth/stencil buffer will be accessed.
		auto subpassDesc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colorAttachment)
			.setPDepthStencilAttachment(&depthStencilAttachment);
		// The depth/stencil buffer is used as part of fragment testing, so those stages need to be included
		auto subpassDep = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);

		std::array attachmentDescriptions = { colorAttachmentDescription, depthStencilAttachmentDescription };

		return GetDevice().createRenderPass(vk::RenderPassCreateInfo()
			.setAttachments(attachmentDescriptions)
			.setSubpasses(subpassDesc)
			.setDependencies(subpassDep)
		);
	}

	inline bool VulkanWrapper::BeginFrame(vk::CommandBuffer& commandBuffer, vk::Framebuffer& framebuffer)
	{
		if (!m_vkbSwapchain.swapchain)
		{
			RebuildSwapchain();
			return false;
		}

		auto& currentFrame = m_frames[m_currentFrameIndex];

		// Wait for our the frame to complete before we start changing it
		// The return value is cast to void because we don't care what it is
		(void)GetDevice().waitForFences(1, &currentFrame.inUse, true, UINT64_MAX);

		// get next image to render into
		uint32_t imageIndex = 0;
		vk::Result acquireResult = vk::Result::eErrorUnknown;
		try
		{
			acquireResult = GetDevice().acquireNextImageKHR(m_vkbSwapchain.swapchain, UINT64_MAX,
				currentFrame.isImageAvailable, {}, &imageIndex);
		}
		catch (const vk::OutOfDateKHRError&)
		{
			acquireResult = vk::Result::eErrorOutOfDateKHR;
		}
		if (acquireResult == vk::Result::eErrorOutOfDateKHR)
		{
			RebuildSwapchain();
			return false;
		}
		m_currentFramebufferIndex = imageIndex;
		auto& targetFramebuffer = m_framebuffers[m_currentFramebufferIndex];

		// make sure our next framebuffer is not in use
		if (targetFramebuffer.inUse)
		{
			// The return value is cast to void because we don't care what it is
			(void)GetDevice().waitForFences(1, &targetFramebuffer.inUse, true, UINT64_MAX);
		}

		targetFramebuffer.inUse = currentFrame.inUse;
		targetFramebuffer.commandBuffer.reset();
		targetFramebuffer.commandBuffer.begin(vk::CommandBufferBeginInfo());

		commandBuffer = targetFramebuffer.commandBuffer;
		framebuffer = targetFramebuffer.framebuffer;
		return true;
	}

	inline void VulkanWrapper::EndFrame()
	{
		auto& currentFrame = m_frames[m_currentFrameIndex];
		auto& targetFramebuffer = m_framebuffers[m_currentFramebufferIndex];

		targetFramebuffer.commandBuffer.end();
			vk::Semaphore waitSemaphores[] =
		{
			currentFrame.isImageAvailable,
		};
		vk::PipelineStageFlags waitFlags[] =
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
		};
		vk::Semaphore signalSemaphores[] =
		{
			currentFrame.isRenderFinished,
		};

		vk::SubmitInfo submitInfo;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = waitFlags;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.signalSemaphoreCount = 1;
		
		vk::CommandBuffer buffers[] = { targetFramebuffer.commandBuffer };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		[[maybe_unused]] auto resetResult = GetDevice().resetFences(1, &currentFrame.inUse);
		[[maybe_unused]] auto submitResult = GetGraphicsQueue().submit(1, &submitInfo, currentFrame.inUse);

		// Present
		//
		// The present command should wait until rendering is finished and this is ensured
		// by having it wait until the semaphores bundled with the submit are signaled

		vk::PresentInfoKHR presentInfo;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.waitSemaphoreCount = 1;

		vk::SwapchainKHR swapchains[] = { m_vkbSwapchain.swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &m_currentFramebufferIndex;

		auto presentResult = vk::Result::eErrorUnknown;
		try
		{
			presentResult = GetPresentQueue().presentKHR(presentInfo);
		}
		catch (const vk::OutOfDateKHRError&)
		{
			presentResult = vk::Result::eErrorOutOfDateKHR;
		}
		if (presentResult == vk::Result::eErrorOutOfDateKHR)
		{
			RebuildSwapchain();
			return;
		}

		m_currentFrameIndex = (m_currentFrameIndex + 1) % kMaxPendingFrames;
	}

	inline bool VulkanWrapper::ExecuteCommands(std::function<void(vk::CommandBuffer&)>&& generateCommands)
	{
		// Create our command buffer to record copy commands
		vk::CommandBufferAllocateInfo cbInfo;
		cbInfo.commandBufferCount = 1;
		cbInfo.commandPool = m_graphicsCommandPool;
		auto commandBuffers = GetDevice().allocateCommandBuffers(cbInfo);
		if (commandBuffers.empty())
			return false;

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandBuffers[0].begin(beginInfo);
		generateCommands(commandBuffers[0]);
		commandBuffers[0].end();

		vk::Fence fence = GetDevice().createFence(vk::FenceCreateInfo());

		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(commandBuffers);
		m_graphicsQueue.submit(submitInfo, fence);

		(void)GetDevice().waitForFences(1, &fence, true, UINT64_MAX);
		GetDevice().destroyFence(fence);
		GetDevice().freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
		return true;
	}

	inline vk::Buffer VulkanWrapper::CreateBuffer(const vk::BufferCreateInfo& createInfo, const void* pInitialData)
	{
		vk::Buffer buffer = GetDevice().createBuffer(createInfo);
		if (!buffer)
			return {};

		vk::MemoryRequirements memoryReq = GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryPropertyFlags memoryFlags{};
		if (!pInitialData)
			memoryFlags |= vk::MemoryPropertyFlagBits::eDeviceLocal;
		else
			memoryFlags |= vk::MemoryPropertyFlagBits::eHostVisible;

		vk::DeviceMemory bufferMemory = AllocateMemory(memoryReq, memoryFlags);
		if (!bufferMemory)
		{
			GetDevice().destroyBuffer(buffer);
			return {};
		}

		GetDevice().bindBufferMemory(buffer, bufferMemory, 0);

		if (pInitialData)
		{
			void* pMapped = GetDevice().mapMemory(bufferMemory, 0, memoryReq.size);
			if (pMapped)
			{
				memset(pMapped, 0, memoryReq.size);
				memcpy(pMapped, pInitialData, createInfo.size);
				GetDevice().flushMappedMemoryRanges(
					vk::MappedMemoryRange()
					.setMemory(bufferMemory)
					.setOffset(0)
					.setSize(VK_WHOLE_SIZE)
				);
				GetDevice().unmapMemory(bufferMemory);
			}
		}

		m_bufferMemoryMap[buffer] = bufferMemory;
		return buffer;
	}

	inline vk::Buffer VulkanWrapper::CreateBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, const void* pInitialData)
	{
		vk::BufferCreateInfo createInfo{};
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = vk::SharingMode::eExclusive;

		// Ensure the buffer is being marked as a transfer destination so we can copy the initial data
		if (!pInitialData)
			createInfo.usage |= vk::BufferUsageFlagBits::eTransferDst;

		return CreateBuffer(createInfo, pInitialData);
	}

	inline void VulkanWrapper::DestroyBuffer(vk::Buffer buffer)
	{
		GetDevice().destroyBuffer(buffer);

		auto bufferIter = m_bufferMemoryMap.find(buffer);
		if (bufferIter != m_bufferMemoryMap.end())
		{
			GetDevice().freeMemory(bufferIter->second);
			m_bufferMemoryMap.erase(bufferIter);
		}
	}

	inline vk::DeviceMemory VulkanWrapper::GetBufferMemoryHandle(vk::Buffer buffer) const
	{
		auto bufferIter = m_bufferMemoryMap.find(buffer);
		if (bufferIter != m_bufferMemoryMap.end())
			return bufferIter->second;
		return nullptr;
	}

	inline vk::Image VulkanWrapper::CreateImage(const vk::ImageCreateInfo& imageInfo, const void* pInitialData, vk::DeviceSize initialDataSize)
	{
		vk::Image image = GetDevice().createImage(imageInfo);
		if (!image)
			return nullptr;

		auto memoryReq = GetDevice().getImageMemoryRequirements(image);
		vk::DeviceMemory imageMemory = AllocateMemory(memoryReq, vk::MemoryPropertyFlagBits::eDeviceLocal);
		if (!imageMemory)
		{
			GetDevice().destroyImage(image);
			return nullptr;
		}

		GetDevice().bindImageMemory(image, imageMemory, 0);

		if (pInitialData && !UploadImageData(image, imageInfo.extent, pInitialData, initialDataSize, imageInfo.arrayLayers))
		{
			GetDevice().destroyImage(image);
			GetDevice().freeMemory(imageMemory);
			return nullptr;
		}

		m_imageMemoryMap[image] = imageMemory;
		return image;
	}

	inline std::tuple<vk::Image, vk::ImageView> VulkanWrapper::CreateImageAndView(const vk::ImageCreateInfo& imageInfo, const void* pInitialData, vk::DeviceSize initialDataSize)
	{
		vk::Image image = CreateImage(imageInfo, pInitialData, initialDataSize);
		if (!image)
			return {};

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.format = imageInfo.format;
		viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eNone;

		// Mild hack to support detect depth/stencil images
		if (imageInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			if (viewInfo.format == vk::Format::eD16Unorm ||
				viewInfo.format == vk::Format::eD16UnormS8Uint ||
				viewInfo.format == vk::Format::eD24UnormS8Uint ||
				viewInfo.format == vk::Format::eD32Sfloat ||
				viewInfo.format == vk::Format::eD32SfloatS8Uint)
			{
				viewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eDepth;
			}

			if (viewInfo.format == vk::Format::eD16UnormS8Uint ||
				viewInfo.format == vk::Format::eD24UnormS8Uint ||
				viewInfo.format == vk::Format::eD32SfloatS8Uint ||
				viewInfo.format == vk::Format::eS8Uint)
			{
				viewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}

		// Fall back to color if none other set
		if ((imageInfo.usage & vk::ImageUsageFlagBits::eColorAttachment) || !viewInfo.subresourceRange.aspectMask)
			viewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eColor;

		// Mild hack to treat an array of 6 2D layers as a cubemap
		if (imageInfo.imageType == vk::ImageType::e2D && imageInfo.arrayLayers == 6)
			viewInfo.viewType = vk::ImageViewType::eCube;
		// Detect standard image types
		else if (imageInfo.imageType == vk::ImageType::e1D)
			viewInfo.viewType = (imageInfo.arrayLayers > 1) ? vk::ImageViewType::e1DArray : vk::ImageViewType::e1D;
		else if (imageInfo.imageType == vk::ImageType::e2D)
			viewInfo.viewType = (imageInfo.arrayLayers > 1) ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
		else if (imageInfo.imageType == vk::ImageType::e3D)
			viewInfo.viewType = vk::ImageViewType::e3D;

		vk::ImageView imageView = GetDevice().createImageView(viewInfo);
		if (!imageView)
		{
			DestroyImage(image);
			return {};
		}

		return { image, imageView };
	}

	inline std::tuple<vk::Image, vk::ImageView> VulkanWrapper::CreateTexture2DImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData, vk::DeviceSize initialDataSize)
	{
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		if (pInitialData)
			usage |= vk::ImageUsageFlagBits::eTransferDst;
		return CreateImageAndView(
			vk::ImageCreateInfo()
				.setImageType(vk::ImageType::e2D)
				.setExtent(vk::Extent3D(extents, 1))
				.setMipLevels(1)
				.setArrayLayers(1)
				.setFormat(format)
				.setUsage(usage),
			pInitialData,
			initialDataSize
		);
	}

	inline std::tuple<vk::Image, vk::ImageView> VulkanWrapper::CreateCubemapImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData, vk::DeviceSize initialDataSize)
	{
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled;
		if (pInitialData)
			usage |= vk::ImageUsageFlagBits::eTransferDst;
		return CreateImageAndView(
			vk::ImageCreateInfo()
				.setImageType(vk::ImageType::e2D)
				.setExtent(vk::Extent3D(extents, 1))
				.setMipLevels(1)
				.setArrayLayers(6)
				.setFormat(format)
				.setUsage(usage)
				.setFlags(vk::ImageCreateFlagBits::eCubeCompatible),
			pInitialData,
			initialDataSize
		);
	}

	inline void VulkanWrapper::DestroyImage(vk::Image image)
	{
		GetDevice().destroyImage(image);

		auto imageIter = m_imageMemoryMap.find(image);
		if (imageIter != m_imageMemoryMap.end())
		{
			GetDevice().freeMemory(imageIter->second);
			m_imageMemoryMap.erase(imageIter);
		}
	}

	inline void VulkanWrapper::DestroyImageAndView(vk::Image image, vk::ImageView view)
	{
		GetDevice().destroyImageView(view);
		DestroyImage(image);
	}

	inline vk::DeviceMemory VulkanWrapper::GetImageMemoryHandle(vk::Image image) const
	{
		auto imageIter = m_imageMemoryMap.find(image);
		if (imageIter != m_imageMemoryMap.end())
			return imageIter->second;
		return nullptr;
	}

	inline bool VulkanWrapper::UploadImageData(vk::Image image, vk::Extent3D extents, const void* pData, vk::DeviceSize dataSize, uint32_t layerCount)
	{
		vk::Buffer stagingBuffer = GetDevice().createBuffer(
			vk::BufferCreateInfo()
				.setSize(dataSize)
				.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
				.setSharingMode(vk::SharingMode::eExclusive)
		);
		if (!stagingBuffer)
			return false;

		auto memoryReq = GetDevice().getBufferMemoryRequirements(stagingBuffer);
		vk::DeviceMemory stagingBufferMemory = AllocateMemory(memoryReq, vk::MemoryPropertyFlagBits::eHostVisible);
		if (!stagingBufferMemory)
		{
			GetDevice().destroyBuffer(stagingBuffer);
			return false;
		}

		GetDevice().bindBufferMemory(stagingBuffer, stagingBufferMemory, 0);
		void* pMapped = GetDevice().mapMemory(stagingBufferMemory, 0, memoryReq.size);
		memset(pMapped, 0, memoryReq.size);
		memcpy(pMapped, pData, dataSize);
		GetDevice().flushMappedMemoryRanges(
			vk::MappedMemoryRange()
				.setMemory(stagingBufferMemory)
				.setOffset(0)
				.setSize(VK_WHOLE_SIZE)
		);
		GetDevice().unmapMemory(stagingBufferMemory);

		ExecuteCommands([&image, &extents, &stagingBuffer, layerCount](vk::CommandBuffer& cb)
		{
			vk::ImageMemoryBarrier barrier{};
			barrier.image = image;
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = layerCount;

			// Ensure the image is transformed on the GPU into a format that is ready to be transferred into
			barrier.oldLayout = vk::ImageLayout::eUndefined;
			barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			cb.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer,
				{}, {}, {}, { barrier });

			// Now perform the copy from our staging buffer
			vk::BufferImageCopy copyInfo{};
			copyInfo.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			copyInfo.imageSubresource.mipLevel = 0;
			copyInfo.imageSubresource.baseArrayLayer = 0;
			copyInfo.imageSubresource.layerCount = layerCount;
			copyInfo.imageExtent = extents;
			cb.copyBufferToImage(stagingBuffer, image, barrier.newLayout, 1, &copyInfo);

			// Transform the image back into a layout ready for use by the shader
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			cb.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				{}, {}, {}, { barrier });
		});

		GetDevice().destroyBuffer(stagingBuffer);
		GetDevice().freeMemory(stagingBufferMemory);
		return true;
	}

	inline std::shared_ptr<GraphicsPipeline> VulkanWrapper::CreatePipeline(const GraphicsPipelineConfig& config)
	{
		// if the render pass in the config is null, it is for use with the display render pass
		// so do not create if the RP is not available
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;

		if (!config.renderPass)
		{
			if (auto pass = GetDisplayRenderPass())
			{
				GraphicsPipelineConfig overrideConfig = config;
				overrideConfig.renderPass = pass;
				std::tie(pipeline, layout) = CreatePipelineAndLayout(overrideConfig);
			}
		}
		else
		{
			std::tie(pipeline, layout) = CreatePipelineAndLayout(config);
		}

		auto wrapper = std::make_shared<GraphicsPipeline>();
		std::get<0>(*wrapper) = pipeline;
		std::get<1>(*wrapper) = layout;
		std::get<2>(*wrapper) = config;
		m_graphicsPipelines.push_back(wrapper);
		return wrapper;
	}

	inline void VulkanWrapper::DestroyPipeline(std::shared_ptr<GraphicsPipeline>& pipeline)
	{
		if (!pipeline)
			return;

		if (auto& pipelineRef = std::get<0>(*pipeline))
		{
			GetDevice().destroyPipeline(pipelineRef);
			pipelineRef = vk::Pipeline();
		}
		if (auto& layout = std::get<1>(*pipeline))
		{
			GetDevice().destroyPipelineLayout(layout);
			layout = vk::PipelineLayout();
		}

		pipeline.reset();
	}

	inline std::tuple<vk::Pipeline, vk::PipelineLayout> VulkanWrapper::CreatePipelineAndLayout(const GraphicsPipelineConfig& config)
	{
		auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setPushConstantRanges(config.shaderPushConstants)
			.setSetLayouts(config.shaderUniforms);

		vk::PipelineLayout layout = GetDevice().createPipelineLayout(layoutCreateInfo);

		auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(config.topology);

		auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(config.vertexBindings)
			.setVertexAttributeDescriptions(config.vertexAttributes);

		auto rasterState = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(config.cullMode)
			.setFrontFace(config.frontFace)
			.setPolygonMode(config.polygonMode)
			.setLineWidth(1)
			.setDepthBiasEnable(config.depthBiasEnable);

		auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(config.depthTestEnable)
			.setDepthWriteEnable(config.depthWriteEnable)
			.setDepthCompareOp(config.depthCompareOp);

		auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(config.colorBlendStates);

		auto dynamicState = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStates(config.dynamicStates);

		auto multisampleState = vk::PipelineMultisampleStateCreateInfo();

		auto viewportState = vk::PipelineViewportStateCreateInfo();

		// If no viewports defined, use the display's viewport
		const std::array<const vk::Viewport, 1> defaultViewports = { m_displayViewport };
		viewportState.setViewports(config.viewports.empty() ? vk::ArrayProxyNoTemporaries<const vk::Viewport>(defaultViewports) : config.viewports);

		// If no scissors defined, match viewport
		const std::array<const vk::Rect2D, 1> defaultScissors =
		{
			vk::Rect2D{
				vk::Offset2D{ static_cast<int32_t>(viewportState.pViewports->x), static_cast<int32_t>(viewportState.pViewports->y) },
				vk::Extent2D{ static_cast<uint32_t>(viewportState.pViewports->width), static_cast<uint32_t>(viewportState.pViewports->height) }
			}
		};
		viewportState.setScissors(config.scissors.empty() ? vk::ArrayProxyNoTemporaries<const vk::Rect2D>(defaultScissors) : config.scissors);


		auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setLayout(layout)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPVertexInputState(&vertexInputState)
			.setStages(config.shaderStages)
			.setPRasterizationState(&rasterState)
			.setPDepthStencilState(&depthStencilState)
			.setPColorBlendState(&colorBlendState)
			.setPMultisampleState(&multisampleState)
			.setPViewportState(&viewportState)
			.setPDynamicState(&dynamicState)
			.setRenderPass(config.renderPass);

		const auto& [result, pipeline] = GetDevice().createGraphicsPipeline(GetPipelineCache(), pipelineCreateInfo);
		if (result != vk::Result::eSuccess)
		{
			GetDevice().destroyPipelineLayout(layout);
			return {};
		}

		return { pipeline, layout };
	}

	inline ShaderUniform VulkanWrapper::CreateUniformForBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset, vk::ShaderStageFlags stages)
	{
		auto type = vk::DescriptorType::eUniformBuffer;

		std::array layoutBindings =
		{
			vk::DescriptorSetLayoutBinding()
				.setBinding(binding)
				.setStageFlags(stages)
				.setDescriptorType(type)
				.setDescriptorCount(1)
		};

		vk::DescriptorSetLayout layout = GetDevice().createDescriptorSetLayout(
			vk::DescriptorSetLayoutCreateInfo()
				.setBindings(layoutBindings)
		);
		if (!layout)
			return {};

		auto sets = GetDevice().allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(m_descriptorPool)
				.setSetLayouts(layout)
				.setDescriptorSetCount(1)
		);
		if (sets.empty())
		{
			GetDevice().destroyDescriptorSetLayout(layout);
			return {};
		}

		if (buffer)
		{
			auto bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(buffer)
				.setOffset(offset)
				.setRange(size);

			auto initialUpdate = vk::WriteDescriptorSet()
				.setDescriptorCount(1)
				.setDescriptorType(type)
				.setDstBinding(binding)
				.setDstSet(sets[0])
				.setBufferInfo(bufferInfo);

			GetDevice().updateDescriptorSets(initialUpdate, {});
		}

		return ShaderUniform{ std::make_tuple(sets[0], layout, type, binding) };
	}

	inline ShaderUniform VulkanWrapper::CreateUniformForImage(uint32_t binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout imageLayout, vk::ShaderStageFlags stages)
	{
		auto type = vk::DescriptorType::eCombinedImageSampler;

		std::array layoutBindings =
		{
			vk::DescriptorSetLayoutBinding()
				.setBinding(binding)
				.setStageFlags(stages)
				.setDescriptorType(type)
				.setDescriptorCount(1)
		};

		vk::DescriptorSetLayout layout = GetDevice().createDescriptorSetLayout(
			vk::DescriptorSetLayoutCreateInfo()
				.setBindings(layoutBindings)
		);
		if (!layout)
			return {};

		auto sets = GetDevice().allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(m_descriptorPool)
				.setSetLayouts(layout)
				.setDescriptorSetCount(1)
		);
		if (sets.empty())
		{
			GetDevice().destroyDescriptorSetLayout(layout);
			return {};
		}

		if (view && sampler)
		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageView(view)
				.setSampler(sampler)
				.setImageLayout(imageLayout);

			vk::WriteDescriptorSet initialUpdate = vk::WriteDescriptorSet()
				.setDescriptorCount(1)
				.setDescriptorType(type)
				.setDstBinding(binding)
				.setDstSet(sets[0])
				.setImageInfo(imageInfo);

			GetDevice().updateDescriptorSets(initialUpdate, {});
		}
		
		return { std::make_tuple(sets[0], layout, type, binding) };
	}

	inline void VulkanWrapper::DestroyUniform(ShaderUniform uniform)
	{
		if (auto set = uniform.GetSet())
			GetDevice().freeDescriptorSets(m_descriptorPool, { set });
		if (auto layout = uniform.GetLayout())
			GetDevice().destroyDescriptorSetLayout(layout);
	}

	inline vk::DescriptorSet VulkanWrapper::CreateAlternateImageValueForUniform(ShaderUniform uniform, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout)
	{
		vk::DescriptorSetLayout setLayout = uniform.GetLayout();

		auto sets = GetDevice().allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(GetDescriptorPool())
				.setDescriptorSetCount(1)
				.setSetLayouts(setLayout)
		);
		if (sets.empty())
			return {};

		vk::WriteDescriptorSet update = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(uniform.GetType())
			.setDstBinding(uniform.GetBinding())
			.setDstSet(sets[0])
			.setImageInfo(
				vk::DescriptorImageInfo()
					.setImageView(view)
					.setSampler(sampler)
					.setImageLayout(layout)
			);
		GetDevice().updateDescriptorSets({ update }, { });
		return sets[0];
	}

	inline void VulkanWrapper::FreeDescriptorSet(vk::DescriptorSet set)
	{
		GetDevice().freeDescriptorSets(GetDescriptorPool(), { set });
	}

	inline RenderTarget VulkanWrapper::CreateRenderTarget(vk::Extent2D extents, vk::Format colorFormat, vk::Format depthFormat)
	{
		if (colorFormat == vk::Format::eUndefined && depthFormat == vk::Format::eUndefined)
			return {};

		RenderTarget target;
		std::vector<vk::AttachmentDescription> attachments;
		std::optional<vk::AttachmentReference> colorReference;
		std::optional<vk::AttachmentReference> depthReference;

		// Subpass dependencies call out how the attachments will be accessed between the different
		// stages of the pipeline being executed. For a render target that will also be used in a shader,
		// we call out two modes of access:
		std::array dependencies =
		{
			// First, starting from before the subpass is executed (VK_SUBPASS_EXTERNAL)
			// we will need to call out what the old (src) stage access was and what the
			// new (dst) access will be when the subpass is active. What is accessed and how is based on
			// what attachments are created, so these will be defined later
			vk::SubpassDependency()
				.setSrcSubpass(VK_SUBPASS_EXTERNAL)
				.setDstSubpass(0)
				.setSrcStageMask(vk::PipelineStageFlagBits::eNone)
				.setDstStageMask(vk::PipelineStageFlagBits::eNone)
				.setSrcAccessMask(vk::AccessFlagBits::eNone)
				.setDstAccessMask(vk::AccessFlagBits::eNone),
			// Once the subpass is done, we call out any access changes that should occur after completion
			// so the access is set up properly for when the attachments are later used by a shader for input
			vk::SubpassDependency()
				.setSrcSubpass(0)
				.setDstSubpass(VK_SUBPASS_EXTERNAL)
				.setSrcStageMask(vk::PipelineStageFlagBits::eNone)
				.setDstStageMask(vk::PipelineStageFlagBits::eNone)
				.setSrcAccessMask(vk::AccessFlagBits::eNone)
				.setDstAccessMask(vk::AccessFlagBits::eNone),
		};

		if (colorFormat != vk::Format::eUndefined)
		{
			auto [image, view] = CreateImageAndView(
				vk::ImageCreateInfo()
					.setFormat(colorFormat)
					.setExtent(vk::Extent3D(extents, 1))
					.setImageType(vk::ImageType::e2D)
					.setInitialLayout(vk::ImageLayout::eUndefined)
					.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment)
					.setSamples(vk::SampleCountFlagBits::e1)
					.setSharingMode(vk::SharingMode::eExclusive)
					.setTiling(vk::ImageTiling::eOptimal)
					.setMipLevels(1)
					.setArrayLayers(1)
			);
			if (!image)
			{
				DestroyRenderTarget(target);
				return {};
			}

			target.images.push_back(image);
			target.views.push_back(view);

			colorReference = vk::AttachmentReference()
				.setAttachment(static_cast<uint32_t>(attachments.size()))
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			attachments.push_back(vk::AttachmentDescription()
				.setFormat(colorFormat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			);

			// We're dealing with colors, so once we're past the fragment shader, we'll be setting up
			// the attachments to be used as a color output, which needs read/write access.
			dependencies[0].srcStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
			dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependencies[0].dstAccessMask |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

			// Once we're done with the subpass, we're done with color output, so we change the access
			// back to what's appropriate for when the attachments get used as shader inputs, so
			// that means the fragment shader will need read access.
			dependencies[1].srcStageMask |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependencies[1].dstStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
			dependencies[1].srcAccessMask |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
			dependencies[1].dstAccessMask |= vk::AccessFlagBits::eShaderRead;
		}

		if (depthFormat != vk::Format::eUndefined)
		{
			auto [image, view] = CreateImageAndView(
				vk::ImageCreateInfo()
					.setFormat(depthFormat)
					.setExtent(vk::Extent3D(extents, 1))
					.setImageType(vk::ImageType::e2D)
					.setInitialLayout(vk::ImageLayout::eUndefined)
					.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment)
					.setSamples(vk::SampleCountFlagBits::e1)
					.setSharingMode(vk::SharingMode::eExclusive)
					.setTiling(vk::ImageTiling::eOptimal)
					.setMipLevels(1)
					.setArrayLayers(1)
			);
			if (!image)
			{
				DestroyRenderTarget(target);
				return {};
			}

			target.images.push_back(image);
			target.views.push_back(view);

			depthReference = vk::AttachmentReference()
				.setAttachment(static_cast<uint32_t>(attachments.size()))
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			attachments.push_back(vk::AttachmentDescription()
				.setFormat(depthFormat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			);

			// We're dealing with depth values, so unlike color we need to mark using the attachments
			// for depth read/write when we do fragment testing.
			dependencies[0].srcStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
			dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
			dependencies[0].dstAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			// Once we're done with the subpass we'll want to read the values in the shader
			// so mark that the fragment shader should be able to read it.
			dependencies[1].srcStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
			dependencies[1].dstStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
			dependencies[1].srcAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			dependencies[1].dstAccessMask |= vk::AccessFlagBits::eShaderRead;
		}

		auto subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		if (colorReference)
		{
			subpass.pColorAttachments = &colorReference.value();
			subpass.colorAttachmentCount = 1;
		}
		if (depthReference)
		{
			subpass.pDepthStencilAttachment = &depthReference.value();
		}

		target.renderPass = GetDevice().createRenderPass(
			vk::RenderPassCreateInfo()
				.setAttachments(attachments)
				.setDependencies(dependencies)
				.setSubpasses(subpass)
		);
		if (!target.renderPass)
		{
			DestroyRenderTarget(target);
			return {};
		}

		target.framebuffer = GetDevice().createFramebuffer(
			vk::FramebufferCreateInfo()
				.setRenderPass(target.renderPass)
				.setAttachments(target.views)
				.setWidth(extents.width)
				.setHeight(extents.height)
				.setLayers(1)
		);
		if (!target.framebuffer)
		{
			DestroyRenderTarget(target);
			return {};
		}

		return target;
	}

	inline void VulkanWrapper::DestroyRenderTarget(RenderTarget target)
	{
		if (target.framebuffer)
			GetDevice().destroyFramebuffer(target.framebuffer);
		if (target.renderPass)
			GetDevice().destroyRenderPass(target.renderPass);

		auto imageIter = target.images.begin();
		auto viewIter = target.views.begin();
		for (; imageIter != target.images.end() && viewIter != target.views.end();
			++imageIter, ++viewIter)
		{
			DestroyImageAndView(*imageIter, *viewIter);
		}
	}

	inline int VulkanWrapper::FindMemoryTypeIndex(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags)
	{
		for (uint32_t typeIndex = 0; typeIndex < m_vkbPhysicalDevice.memory_properties.memoryTypeCount; ++typeIndex)
		{
			if (req.memoryTypeBits & (0x1 << typeIndex))
			{
				vk::MemoryPropertyFlags memoryFlags(m_vkbPhysicalDevice.memory_properties.memoryTypes[typeIndex].propertyFlags);
				if ((flags & memoryFlags) == flags)
				{
					return static_cast<int>(typeIndex);
				}
			}
		}
		return -1;
	}

	inline vk::DeviceMemory VulkanWrapper::AllocateMemory(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags)
	{
		return GetDevice().allocateMemory(
			vk::MemoryAllocateInfo()
				.setAllocationSize(req.size)
				.setMemoryTypeIndex(FindMemoryTypeIndex(req, flags))
		);
	}

}
