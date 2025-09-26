#pragma once
#include "../HelperFiles/MathAlias.h"

namespace nest
{
	struct ManualMovement
	{
		Vec2F m_endLocation = { -1.f, -1.f };
		float m_timeBeforePlay = .0f;
		float m_speedMultiplier = .0f;
		bool m_stopTracking = false;
		ManualMovement(float x, float y, bool stopTracking = false)
		{
			m_endLocation = { x, y };
			m_stopTracking = stopTracking;
		}
	};
}