#pragma once
#pragma warning(disable : 4324)
#include "../Configuration/EngineConfig.h"
#include "../Math/MathAlias.h"
/*
We only need to align data individually if the struct or uniform will be stored in a buffer
else we just need to make sure the struct is a multiple of 16
pages 137 - 139 in openGL docs, also refer to website for more info
https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout
*/
namespace nest
{
	// mesh data
	struct Material
	{
		enum OptionFlags : uint8_t
		{
			eNone = 0x0,
			eLit = 0x01 << 0,	
			eCastShadows = 0x01 << 1,
			eCatchShadows = 0x01 << 2
		};
		enum TextureFlags : uint8_t
		{
			eNoTexture = 0x0,
			eDiffuse = 0x01 << 0,
			eNormal = 0x01 << 1,
			eRoughness = 0x01 << 2,
			eMetallic = 0x01 << 3,
			eAO = 0x01 << 4,
			eOpacity = 0x01 << 5
		};

		Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float metallic = .5f;
		float roughness = .5f;
#if _ENABLE_LIGHTING == 1
		uint32_t enableLighting = true;
#else
		uint32_t enableLighting = false;
#endif
		uint32_t hasTexture = false;
		uint32_t hasTransparency = false;
		uint32_t textureFlags = eNoTexture;
		int textureId = -1;
		int textureRepeatCount = 1;
#if _RENDER_SHADOWS == 1
		uint32_t options = eLit | eCastShadows | eCatchShadows;
#else
		uint32_t options = eLit;
#endif
		float shadowBias = 0.005f;
	};
	struct MeshVertex 
	{
		Vec3 position{};
		alignas(16) Vec3 normal{0.f, 1.f, 0.f};
		alignas(16) Vec2 texcoord0{};
		alignas(16) Vec3 tangent{ 0.f, 1.f, 0.f };
		alignas(16) Vec3 bitangent{ 0.f, 1.f, 0.f };
	};

	struct Light
	{
		enum Type : uint32_t
		{
			ePoint,
			eSun,
			eSpot,
			eArea
		};
		uint32_t type = ePoint;
		alignas(16) Vec3 diffuse{};
		alignas(16) Vec3 position{};
		alignas(16) Vec3 attenuation{};
		alignas(16) Vec3 direction{};
		alignas(16) Vec3 up{};
		alignas(16) Vec2 size{};
		float lightRange = 50.f; // How far the light will go in a single direction
		// spotlight data
		float innerCone = 0.f;
		float outerCone = 0.f;
		float cutoff = 0.0f;
		//------------------------
		alignas(16) Mat4 viewProj = glm::identity<glm::mat4>();
	};
	
	// constants
	struct PushConstants
	{
		Mat4 objectMatrix = glm::identity<glm::mat4>();
		nest::Material objectMaterial{};
	};

	struct ShadingConstants
	{


	};

	struct CameraConstants
	{
		static constexpr uint32_t binding = 0;

		Mat4 projectionMatrix = glm::identity<Mat4>();
		Mat4 viewMatrix = glm::identity<Mat4>();
		Vec3 cameraPosition; // used in Blinn-Phong calculation
		alignas(16) Light light0{};
	};

	struct LightConstants
	{
		static constexpr uint32_t binding = 1;

		nest::Light lights[1]{};
		int lightCount = 1;
	};

	struct ShadowPushConstants
	{
		Mat4 light0Matrix = glm::identity<Mat4>();
		Mat4 objectMatrix = glm::identity<Mat4>();
	};
}