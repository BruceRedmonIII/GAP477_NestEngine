#include "CameraManager.h"
#include "../Engine/Engine.h"
#include "../Managers/GraphicsSystem.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../Objects/CameraObject.h"
#include "../VulkanWrappers/Vulkan.h"
#include <cassert>

GAP311::ShaderUniform nest::CameraManager::s_cameraUniform{};
vk::Buffer nest::CameraManager::s_cameraConstantsBuffer{};

bool nest::CameraManager::Init()
{
	m_pActiveCamera = BLEACH_NEW(CameraObject);
	m_pActiveCamera->Init();
	s_cameraConstantsBuffer = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformBuffer(sizeof(CameraConstants));
	s_cameraUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForBuffer(CameraConstants::binding, s_cameraConstantsBuffer, sizeof(CameraConstants));
	return true;
}

void nest::CameraManager::Exit()
{
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_cameraUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyBuffer(s_cameraConstantsBuffer);
	BLEACH_DELETE(m_pActiveCamera);
}

void nest::CameraManager::Update(float deltaTime)
{
	m_pActiveCamera->Update(deltaTime);
}

nest::CameraConstants nest::CameraManager::GetActiveCameraUniform() const
{
	assert(m_pActiveCamera);
	return m_pActiveCamera->GetCameraUniform();
}

nest::CameraConstants& nest::CameraManager::GetActiveCameraUniformRef()
{
	assert(m_pActiveCamera);
	return m_pActiveCamera->GetCameraUniform();
}

void nest::CameraManager::SetActiveCamera(nest::CameraObject* camera)
{
	if (camera)
		m_pActiveCamera = camera;
}

void nest::CameraManager::SetCameraTransform(const Mat4& mat)
{
	if (m_pActiveCamera)
		m_pActiveCamera->SetTransform(mat);
}

void nest::CameraManager::SetCameraUniform(CameraConstants uniform)
{
	m_pActiveCamera->SetCameraUniform(uniform);
}

void nest::CameraManager::ProcessInput() const
{
	m_pActiveCamera->ProcessInput();
}

void nest::CameraManager::UpdateVKBuffer(vk::CommandBuffer& commands)
{
	commands.updateBuffer(s_cameraConstantsBuffer, 0, sizeof(CameraConstants), &m_pActiveCamera->GetCameraUniform());
}
