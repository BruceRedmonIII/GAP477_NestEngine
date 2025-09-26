#include "Scene.h"
#include "../Engine/Engine.h"
#include "../Managers/GraphicsSystem.h"
#include "../Managers/LightManager.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../Nodes/Node.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../Managers/CameraManager.h"
#include "../Managers/EventManager.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Objects/GameObject.h"
#include "../Objects/CameraObject.h"
#include "../Objects/Components/TransformComponent3D.h"
#include "../Objects/Components/KinematicComponent3D.h"
#include "../Objects/Components/CameraComponent.h"
#include <assimp/scene.h>
#include <assimp/types.h>
#include <iostream>

bool nest::Scene::Init(HashedId id)
{
    m_root = BLEACH_NEW(nest::SceneNode());
	m_sceneId = id;
    return true;
}

void nest::Scene::Exit()
{
	// delete root node (deletes all children through destructor)
    BLEACH_DELETE(m_root);
}

void nest::Scene::PreRender() const
{
	m_root->QueRender();
}

std::vector<nest::LightNode*> nest::Scene::GetLights() const
{
    std::vector<LightNode*> lights;
    for (auto node : m_root->GetChildren())
    {
        if (node->kHashedId == LightNode::kHashedId)
        {
            lights.emplace_back(static_cast<LightNode*>(node));
        }
    }
    return lights;
}

void nest::Scene::AddLightNode(LightNode* light)
{
    assert(m_root);
    m_root->EmplaceNode(light);
}

void nest::Scene::AddCameraNode(CameraNode* camera)
{
    assert(m_root);
    m_root->EmplaceNode(camera);
}
