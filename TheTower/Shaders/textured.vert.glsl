#version 450
const uint eMaterialLit = 0x01 << 0;
const uint eMaterialCastShadows = 0x01 << 1;
const uint eMaterialCatchShadows = 0x01 << 2;
//const uint eNormal = 0x01 << 1;

struct Material 
{
    vec4 color;
    float shininess;
    bool enableLighting;
    bool hasTexture;
	uint textureFlags;
	int textureId;
	int textureRepeatCount;
	uint options;
    float shadowBias;
};

struct Light
{
   	uint type;
	vec3 diffuse;
	vec3 position;
	vec3 attenuation;
	vec3 direction;
	vec2 size;
	float innerCone;
	float outerCone;
	mat4 viewProj;
};

layout(push_constant) uniform Constants
{
    mat4 objectMatrix;
    Material objectMaterial;
};

layout(binding = 0) uniform Camera
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 cameraPosition; // needed for Blinn-Phong calculation
	Light light0;
};

//layout(set = 4, binding = 4) uniform sampler2D normalTexture;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord0;

layout(location = 0) smooth out vec3 outPosition;
layout(location = 1) smooth out vec3 outNormal;
layout(location = 2) out vec4 outShadowCoord;
layout(location = 3) out vec2 outTexcoord0;
layout(location = 4) smooth out vec3 outTestNormal;

void main()
{	
	const mat4 biasMat = mat4( 
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5 - objectMaterial.shadowBias, 1.0
    );
    outPosition = (objectMatrix * vec4(inPosition, 1)).xyz;
    outNormal = (objectMatrix * vec4(inNormal, 0)).xyz;
	outTexcoord0 = inTexcoord0 * objectMaterial.textureRepeatCount;
    outShadowCoord = (biasMat * light0.viewProj) * vec4(inPosition, 1);
    gl_Position = projectionMatrix * viewMatrix * vec4(outPosition, 1);
}