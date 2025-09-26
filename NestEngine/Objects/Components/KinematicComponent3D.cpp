#include "KinematicComponent3D.h"
#include "../../Math/MathAlias.h"
#include "../../GlobalConstants/constants.h"
#include "TransformComponent3D.h"
#include "../../Managers/EventManager.h"
#include "../../Managers/InputManager.h"
#include "../GameObject.h"
#include "../../Math/MathAlias.h"
#include "../../Engine/Engine.h"
bool nest::KinematicComponent3D::Init(GameObject* owner)
{
	m_owner = owner;
	if (!m_owner)
		return false;
	m_transform = m_owner->GetComponent<TransformComponent3D>();
	if (!m_transform)
		return false;
	return true;
}

void nest::KinematicComponent3D::Update(float deltaTime)
{
	m_kins.m_linearVelocity += m_kins.m_linearAcceleration;
	m_kins.m_rotationVelocity += m_kins.m_rotationAcceleration;
	UpdateMove(deltaTime);
	ClampLinearSpeed();
	ClampRotationSpeed();
	ResetAcceleration();
}

void nest::KinematicComponent3D::ProcessInput()
{
	InputManager* manager = nest::Engine::GetManager<InputManager>();
	Forwards(manager->KeyPressed(Keycodes::Key_W));
	Backwards(manager->KeyPressed(Keycodes::Key_S));
	Left(manager->KeyPressed(Keycodes::Key_A));
	Right(manager->KeyPressed(Keycodes::Key_D));
	Up(manager->KeyPressed(Keycodes::Key_Space));
	Down(manager->KeyPressed(Keycodes::Key_LeftCtrl));
	if (m_rotateWithMouse)
	{
		ProcessMouseInput();
	}
}

void nest::KinematicComponent3D::UpdateMove(float deltaTime)
{
	glm::mat4 rotation = m_transform->GetRotationMatrix();
	m_transform->SetRotation(rotation);

	m_transform->AddPosition(m_kins.m_linearVelocity * deltaTime);
}

void nest::KinematicComponent3D::ProcessMouseInput()
{
	InputManager* manager = nest::Engine::GetManager<InputManager>();

	// important note here (to future self)
	// sdl will contain data based on the event called
	// aka SDL_MOUSEMOTION will contain motion data, SDL_BUTTONDOWN will not
	if (manager->KeyPressed(Keycodes::Key_RightMouse) && m_mousePressed == false)
	{
			m_mousePressed = true;
			Vec2 mousePos = manager->GetMousePos();
			m_mouseX = mousePos.x;
			m_mouseY = mousePos.y;
	}
	else if (!manager->KeyPressed(Keycodes::Key_RightMouse) && m_mousePressed == true)
	{
		m_mousePressed = false;
		m_kins.m_rotationAcceleration.x = m_kins.m_rotationAcceleration.y = 0.f;
		m_kins.m_rotationVelocity.x = m_kins.m_rotationVelocity.y = 0.f;
	}

	if (m_mousePressed == true)
	{
		auto mousePos = manager->GetMousePos();
		if ((m_mouseX != mousePos.x) || (m_mouseY != mousePos.y))
		{
			float mouseXDiff = mousePos.x - m_mouseX;
			float mouseYDiff = mousePos.y - m_mouseY;
			m_mouseX = mousePos.x;
			m_mouseY = mousePos.y;
			float pitch = mouseYDiff * m_mouseSensitivity;
			float yaw = mouseXDiff * m_mouseSensitivity;
			m_transform->AddRotation(yaw, pitch);
		}
	}
}

void nest::KinematicComponent3D::ClampRotationSpeed()
{
	m_kins.m_rotationAcceleration = glm::clamp(m_kins.m_rotationAcceleration, -m_kins.m_maxRotionAcceleration, m_kins.m_maxRotionAcceleration);
	m_kins.m_rotationVelocity = glm::clamp(m_kins.m_rotationVelocity, -m_kins.m_maxRotationSpeed, m_kins.m_maxRotationSpeed);
}

void nest::KinematicComponent3D::ClampLinearSpeed()
{
	m_kins.m_linearAcceleration = glm::clamp(m_kins.m_linearAcceleration, -m_kins.m_maxLinearAcceleration, m_kins.m_maxLinearAcceleration);
	m_kins.m_linearVelocity = glm::clamp(m_kins.m_linearVelocity, -m_kins.m_maxLinearSpeed, m_kins.m_maxLinearSpeed);
}

void nest::KinematicComponent3D::ResetAcceleration()
{
	m_kins.m_linearAcceleration.x = m_kins.m_linearAcceleration.y = m_kins.m_linearAcceleration.z = 0.f;
	m_kins.m_rotationAcceleration.x = m_kins.m_rotationAcceleration.y = m_kins.m_rotationAcceleration.z = 0.f;
}

// true will move object Forwards
// false will check if object was moving Forwards and stop it if it was
// we do this check in order to allow smooth transitional movement
void nest::KinematicComponent3D::Forwards(bool status)
{
	if (!status && m_kins.m_linearVelocity.z > 0.f)
		m_kins.m_linearAcceleration.z = m_kins.m_linearVelocity.z = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.z += m_kins.m_maxLinearSpeed;
}

// true will move object Backwards
// false will check if object was moving Backwards and stop it if it was
// we do this check in order to allow smooth transitional movement 
void nest::KinematicComponent3D::Backwards(bool status)
{
	if (!status && m_kins.m_linearVelocity.z < 0.f)
		m_kins.m_linearAcceleration.z = m_kins.m_linearVelocity.z = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.z -= m_kins.m_maxLinearSpeed;
}

// true will move object Right
// false will check if object was moving Right and stop it if it was
// we do this check in order to allow smooth transitional movement
void nest::KinematicComponent3D::Right(bool status)
{
	if (!status && m_kins.m_linearVelocity.x > 0.f)
		m_kins.m_linearAcceleration.x = m_kins.m_linearVelocity.x = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.x += m_kins.m_maxLinearSpeed;
}

// true will move object Left
// false will check if object was moving Left and stop it if it was
// we do this check in order to allow smooth transitional movement
void nest::KinematicComponent3D::Left(bool status)
{
	if (!status && m_kins.m_linearVelocity.x < 0.f)
		m_kins.m_linearAcceleration.x = m_kins.m_linearVelocity.x = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.x -= m_kins.m_maxLinearSpeed;
}

// true will move object Up
// false will check if object was moving Up and stop it if it was
// we do this check in order to allow smooth transitional movement
void nest::KinematicComponent3D::Up(bool status)
{
	if (!status && m_kins.m_linearVelocity.y > 0.f)
		m_kins.m_linearAcceleration.y = m_kins.m_linearVelocity.y = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.y += m_kins.m_maxLinearSpeed;
}

// true will move object Down
// false will check if object was moving Down and stop it if it was
// we do this check in order to allow smooth transitional movement
void nest::KinematicComponent3D::Down(bool status)
{
	if (!status && m_kins.m_linearVelocity.y < 0.f)
		m_kins.m_linearAcceleration.y = m_kins.m_linearVelocity.y = 0.f;
	else if (status)
		m_kins.m_linearAcceleration.y -= m_kins.m_maxLinearSpeed;
}
