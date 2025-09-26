#pragma once
#include "../Math/MathAlias.h"
namespace nest
{
	static constexpr Vec3 s_kDefaultPos{ 0.0f, 2.0f, -2.0f };
	static constexpr Vec3 s_kDefaultDir{ 0.0f, 0.0f, 0.0f };
	static constexpr Vec3 s_kDefaultRot{ 0.0f, 1.0f, 0.0f };
	static constexpr Vec3 s_kDefaultUp { 0.f,  1.f,  0.f };
    struct CameraData
    {
		float mouseSensitivity = .015f;
		float yaw = 0.f;
		float pitch = 0.f;
		float roll = 0.f;
		Vec3 position{ 0.f, 2.f, -2.f };
		Vec3 forward{ 0.f, 0.f, 1.f };
		Vec3 up{ 0.f, 1.f, 0.f };
		Vec3 right{ 1.f, 0.f, 0.f };
    };
}