#pragma once
#pragma warning(disable : 4324)
#include "Manager.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../SDLWrappers/Keybind.h"
#include <vector>
namespace vk
{
	class Sampler;
	struct Extent2D;
}
namespace nest
{
	class LightNode;
	class Scene;
	const enum Controls2 : size_t
	{
		Key_L,
		Key_KeyCount
	};
	class LightManager : public Manager
	{
		int m_maxLightCount = 1;
		int m_currentLightCount = 0;
		std::vector<nest::LightNode*> m_lightNodes;
	public:
		static GAP311::ShaderUniform s_lightUniform;
		static GAP311::ShaderUniform s_shadowMapUniform;
		static vk::Buffer s_lightConstantsBuffer;
		static vk::Extent2D s_lightMapDimensions;
		static GAP311::RenderTarget s_targetShadowMap;
		static vk::Sampler s_samplerShadowMap;
	public:
		SET_HASHED_ID(LightManager)
		nest::KeyBinder m_keyBinds[Controls2::Key_KeyCount]{};
		LightConstants m_uniform;
		std::vector<nest::LightNode*> GetLights() { return m_lightNodes; }
		bool Init() override;
		void Exit() override;
		// used only for debug
		void LoadLightsFromScene(const Scene* scene);
		bool AddLight(LightNode* light);
		void UpdateVKBuffer(vk::CommandBuffer& commands) override;
		~LightManager() override = default;
	};
}