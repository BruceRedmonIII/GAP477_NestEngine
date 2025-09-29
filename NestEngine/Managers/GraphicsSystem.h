#pragma once
#include "Manager.h"
#include "../GlobalConstants/constants.h"
#include <unordered_map>
#include <vector>
namespace nest
{
	struct Vulkan;
	class MeshNode;
	struct Pipeline;
	class CameraObject;
	class Scene;
	class LightManager;
	struct Texture;
	struct Material;
	class CameraComponent;
	struct ShadowSettings
	{
		float depthBiasConstant = 1.25f;
		float depthBiasSlope = 1.75f;
		float lightFOV = 90.0f;
		float lightNear = .1f;
		float lightFar = 100.f;
		float lightAspect = s_kWindowParams.z / s_kWindowParams.w;
	};
	class GraphicsSystem
	{
	public:
		Vulkan* m_vulkanWrapper = nullptr;
		ShadowSettings m_shadowSettings;
	private:
		std::vector<nest::Pipeline*> m_pPipelines;
		std::vector<MeshNode*> m_renderQueue;
	public:
		SET_HASHED_ID(GraphicsSystem)
		GraphicsSystem() = default;
		bool Init();
		void Exit();
		void Render();
		void QueRender(MeshNode* node);
		const nest::Pipeline* GetPipeline(HashedId id);
		bool InitPipelines(nest::Scene* scene);
		void RebuildMeshPipelines();
		ShadowSettings& GetShadowSettings() { return m_shadowSettings; }
	private:
		bool InitVulkanWrapper();
		bool InitShaders();
		void RenderGrid(vk::CommandBuffer& commands);
		void RenderShadows(vk::CommandBuffer& commands);
		void SortQue();
	};
}