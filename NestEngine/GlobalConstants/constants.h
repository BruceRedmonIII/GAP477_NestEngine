#pragma once
#include "../Uniforms/MeshDataStructs.h"
#include "../HelperFiles/IdTypes.h"
#include "../Math/MathAlias.h"
#include "../Math/Hash.h"
#include <cstddef>
static constexpr const char* s_kGridVert = "grid.vert";
static constexpr const char* s_kGridFrag = "grid.frag";
static constexpr const char* s_kMeshVert = "mesh.vert";
static constexpr const char* s_kMeshFrag = "mesh.frag";
static constexpr const char* s_kSkyboxVert = "skybox.vert";
static constexpr const char* s_kSkyboxFrag = "skybox.frag";
static constexpr const char* s_kMainName = "main";

// the file returned didn't have the location, so this is a very lazy way of telling which file is which
// TODO: obviously fix this..

static constexpr const char* s_kAssetPath = "Assets/PBR/";
static constexpr const char* s_kShaderPath = "Shaders/";
static constexpr const char* s_kSkyboxPath = "Assets/Skybox/";
static constexpr const char* s_kAppName = "The Tower";
static inline const Vec4 s_kWindowParams{ 0, 0, 1280, 720 };

// Camera settings
static inline const float s_kAspect = s_kWindowParams.z / s_kWindowParams.w;
static inline const float s_kFOV = glm::radians(60.f); // 60 degrees in radians
static inline const float s_kNear = .1f;
static inline const float s_kFar = 512.f;
//

static constexpr const char* s_kAssetPaths[2] =
{
	"Assets/",
	""
};
static constexpr const char* s_kXMLPaths[2] =
{
	"XMLFiles/",
	""
};
static constexpr const char* s_kSkyboxPaths[2] =
{
	"Assets/Skybox/",
	""
};

static constexpr const int s_kFrameRate = 60;

// pipeline constants for loading pipeline config
static constexpr HashedId s_kMeshPipeline = nest::Hash("MeshPipeline");
static constexpr HashedId s_kTexturedMeshPipeline = nest::Hash("TexturedMeshPipeline");
static constexpr HashedId s_kShadowMapPipeline = nest::Hash("ShadowMapPipeline");
static constexpr HashedId s_kGridPipeline = nest::Hash("GridPipeline");
static constexpr HashedId s_kSkyboxPipeline = nest::Hash("SkyboxPipeline");

static constexpr HashedId s_kMeshGroupNode = nest::Hash("MeshGroupNode");
static constexpr HashedId s_kBaseGroupNode = nest::Hash("BaseGroupNode");

static constexpr HashedId s_kCamera = nest::Hash("Camera");
static constexpr HashedId s_kSkyboxKey = nest::Hash("Skybox");
static constexpr HashedId s_kAssetKey = nest::Hash("Assets");
static constexpr HashedId s_kXMLFilesKey = nest::Hash("XMLFiles");
//static inline constexpr const vk::ClearValue s_clearValues[2] = // the color to clear the area with
//{
//	vk::ClearColorValue(0.05f, 0.05f, 0.05f, 0.0f),
//	vk::ClearDepthStencilValue(1.0f, 0),
//};