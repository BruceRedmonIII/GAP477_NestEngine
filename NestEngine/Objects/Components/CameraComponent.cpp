#include "CameraComponent.h"
#include "../../GlobalConstants/constants.h"
#include "TransformComponent3D.h"
#include "../GameObject.h"
#include "../../Math/MathAlias.h"

bool nest::CameraComponent::Init(GameObject* owner)
{
	m_uniform.projectionMatrix = glm::perspective(s_kFOV, s_kAspect, 0.1f, 512.0f);
	m_uniform.projectionMatrix[1][1] *= -1.0f;
	m_uniform.viewMatrix = glm::lookAt(
		s_kDefaultPos,
		s_kDefaultDir,
		s_kDefaultRot
	);
	m_owner = owner;
	if (!m_owner)
		return false;
	m_transform = m_owner->GetComponent<TransformComponent3D>();
	return true;
}

void nest::CameraComponent::Update([[maybe_unused]] float deltaTime)
{
	m_uniform.viewMatrix = GetViewMatrix();
	m_uniform.cameraPosition = m_transform->GetPosition();
}

void nest::CameraComponent::HandleEvent([[maybe_unused]] const Event& evt)
{

}

Mat4 nest::CameraComponent::GetViewMatrix() const
{
	return m_transform->LookAt();
}

void nest::CameraComponent::SetTransform(Mat4 worldMatrix)
{
	m_transform->SetTransform(worldMatrix);
}
