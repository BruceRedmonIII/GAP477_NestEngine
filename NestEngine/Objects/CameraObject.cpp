#include "CameraObject.h"
#include "Components/CameraComponent.h"
#include "Components/KinematicComponent3D.h"
#include "Components/TransformComponent3D.h"
#include "../Managers/CameraManager.h"
#include "../Managers/EventManager.h"
#include "../Engine/Engine.h"
#include "../Math/MathAlias.h"

nest::CameraObject::~CameraObject()
{
	//
}

bool nest::CameraObject::Init()
{
	GameObject::Init();

    // Add TransformComponent
    TransformComponent3D* transformComponent = BLEACH_NEW(TransformComponent3D());
    if (!transformComponent->Init(this))
        return false;
	AddComponent(transformComponent);

    // Add CameraComponent  
    CameraComponent* cameraComponent = BLEACH_NEW(CameraComponent()); 
    if (!cameraComponent->Init(this))
        return false;  
	AddComponent(cameraComponent);
	m_camera = cameraComponent;

    // Add KinsComponent
    KinematicComponent3D* kinsComponent = BLEACH_NEW(KinematicComponent3D());
    if (!kinsComponent->Init(this))
        return false;
	AddComponent(kinsComponent);
	kinsComponent->RotateWithMouse(true);
	m_kins = kinsComponent;

    return true;  
}

nest::CameraConstants& nest::CameraObject::GetCameraUniform()
{
	CameraComponent* cameraComponent = GetComponent<CameraComponent>();
	assert(cameraComponent);
	return cameraComponent->GetUniform();
}

void nest::CameraObject::SetCameraUniform(const CameraConstants& uniform)
{
	CameraComponent* cameraComponent = GetComponent<CameraComponent>();
	if (cameraComponent)
	{
		cameraComponent->SetUniform(uniform);
	}
	else
	{
		_LOG(LogType::kError, "CameraObject does not have a CameraComponent!");
	}
}

void nest::CameraObject::SetTransform(const Mat4& transform)
{
	auto* transformComponent = GetComponent<TransformComponent3D>();
	if (transformComponent)
	{
		transformComponent->SetTransform(transform);
	}
	else
	{
		_LOG(LogType::kError, "CameraObject does not have a TransformComponent3D!");
	}
}

void nest::CameraObject::Forwards(bool status)
{
	m_kins->Forwards(status);
}

void nest::CameraObject::Backwards(bool status)
{
	m_kins->Backwards(status);
}

void nest::CameraObject::Left(bool status)
{
	m_kins->Left(status);
}

void nest::CameraObject::Right(bool status)
{
	m_kins->Right(status);
}

void nest::CameraObject::Up(bool status)
{
	m_kins->Up(status);
}
void nest::CameraObject::Down(bool status)
{
	m_kins->Down(status);
}