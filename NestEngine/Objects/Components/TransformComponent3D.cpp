#include "TransformComponent3D.h"

bool nest::TransformComponent3D::Init(GameObject* owner)
{
    m_pOwner = owner;
    if (!m_pOwner)
        return false;
    return true;
}

Mat4 nest::TransformComponent3D::GetRotationMatrix() const
{
	return glm::yawPitchRoll(m_yaw, m_pitch, m_roll);
}

Mat4 nest::TransformComponent3D::LookAt() const
{
	return glm::lookAt(m_position, m_position + m_forward, m_up);
}

void nest::TransformComponent3D::SetTransform(const Mat4& mat)
{
	m_position = mat[3];
	m_right = Vec3(mat[0]); // X-axis
	m_up = Vec3(mat[1]); // Y-axis
	m_forward = Vec3(mat[2]); // Z-axis (forward is -Z)
	// Extract rotation
	Mat3 rotationMatrix = glm::extractMatrixRotation(mat);
	Quat rotationQuat = glm::quat_cast(rotationMatrix);

	// Get Euler angles in radians
	Vec3 euler = glm::eulerAngles(rotationQuat);

	// Store in degrees (or leave in radians if you prefer)
	m_pitch = euler.x; // Rotation around X
	m_yaw = euler.y; // Rotation around Y
	m_roll = euler.z;// Rotation around Z
}

void nest::TransformComponent3D::SetPosition(const Vec3& position)
{
	m_transform[3][0] = position.x;
	m_transform[3][1] = position.y;
	m_transform[3][2] = position.z;
}

void nest::TransformComponent3D::SetRotation(const Mat4& rotation)
{
	m_right = Vec3(rotation *   Vec4(1.f, 0.f, 0.f, 0.f));
	m_up = Vec3(rotation *	    Vec4(0.f, 1.f, 0.f, 0.f));
	m_forward = Vec3(rotation * Vec4(0.f, 0.f, 1.f, 0.f));
}

void nest::TransformComponent3D::AddRotation(float yaw, float pitch)
{
	m_yaw += yaw;
	m_pitch += pitch;
	m_yaw = glm::wrapAngle(m_yaw);
	m_pitch = glm::wrapAngle(m_pitch);
	SetRotation(GetRotationMatrix());

}

void nest::TransformComponent3D::SetScale(const Vec3& scale)
{
	m_transform = glm::scale(m_transform, scale);
}

void nest::TransformComponent3D::AddPosition(const Vec3& position)
{
	m_position += (position.z * m_forward) +
				  (position.x * m_right) +
				  (position.y * m_up);
	SetPosition(m_position);
}
