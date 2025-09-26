#pragma once
#include "../Math/MathAlias.h"

namespace nest
{
	struct Kinematics
	{
		// movement
		// these values will update the cameras movement
		// acceleration gets added to velocity every frame
		// if no buttons are pressed, velocity is hard set to 0
		// NOTE: There is no deceleration currently, only acceleration
		Vec3 m_linearVelocity{ 0.f, 0.f, 0.f };
		Vec3 m_rotationVelocity{ 0.f, 0.f, 0.f };
		Vec3 m_rotationAcceleration{ 0.f, 0.f, 0.f };
		Vec3 m_linearAcceleration{ 0.f, 0.f, 0.f };

		// Max values
		// these double as the additives
		float m_maxLinearSpeed = 16.f;
		float m_maxLinearAcceleration = 4.f;
		float m_maxRotationSpeed = 1.f;
		float m_maxRotionAcceleration = 1.f;;
	};
}