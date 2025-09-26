#pragma once
#include "../Math/Hash.h"
#include <vector>
union SDL_Event;
namespace GAP311
{
	struct ShaderUniform;
}
namespace vk
{
	class Buffer;
}
namespace nest
{
	class SceneNode;
	class BaseNode;
	class MeshNode;
	class LightManager;
	class CameraObject;
	class LightNode;
	class CameraNode;
	class Scene
	{
	private:
		SceneNode* m_root = nullptr;
		HashedId m_sceneId;
	public:
		~Scene() = default;
		bool Init(HashedId id);
		HashedId GetId() { return m_sceneId; }
		void Exit();
		void PreRender() const;
		SceneNode* GetRoot() { return m_root; }
		std::vector<LightNode*> GetLights() const;
		void AddLightNode(LightNode* light);
		void AddCameraNode(CameraNode* camera);
	};
}
