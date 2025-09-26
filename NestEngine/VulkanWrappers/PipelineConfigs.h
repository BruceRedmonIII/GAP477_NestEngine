#pragma once
#include "../lib/gap-311/VulkanWrapper.hpp"
namespace nest
{
	static inline vk::FrontFace s_frontFace = vk::FrontFace::eCounterClockwise;
	static inline bool s_depthTest = true;
	static inline vk::CompareOp s_compareOp = vk::CompareOp::eLess;
	static inline bool s_depthWrite = true;
	static inline vk::CullModeFlagBits s_cullMode = vk::CullModeFlagBits::eBack;
	static inline vk::PolygonMode s_polygonMode = vk::PolygonMode::eFill;

	struct Pipeline;
	class LightManager;

	// These are used for code clean up / readability and are not intended to be generalized functions.
	
	// Creates a preset mesh pipeline config.
	nest::Pipeline* CreateMeshPipeline();

	// Creates a preset Grid pipeline config.
	nest::Pipeline* CreateGridPipeline();

	// Creates a preset pipeline specifically for rendering meshes that have textures
	nest::Pipeline* CreateTexturedMeshPipeline();

	// Creates a pipeline to generate shadow maps based on light and object position
	nest::Pipeline* CreateShadowMapPipeline();

	nest::Pipeline* CreateSkyboxPipeline();

	void RebuildMeshPipeline(nest::Pipeline* pipeline);
	void RebuildTexturedMeshPipeline(nest::Pipeline* pipeline);
}