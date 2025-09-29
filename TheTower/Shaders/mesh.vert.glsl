#version 450
const uint eMaterialLit = 0x01 << 0;
const uint eMaterialCastShadows = 0x01 << 1;
const uint eMaterialCatchShadows = 0x01 << 2;

struct Material 
{
    vec4 color;
	float metallic;
	float roughness;
    bool enableLighting;
    bool hasTexture;
	bool hasTransparency;
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
	vec3 up;
	vec2 size;
	float lightRange;
	float innerCone;
	float outerCone;
	float cutoff;
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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 0) smooth out vec3 outPosition;
layout(location = 1) smooth out vec3 outNormal;
layout(location = 2) out vec4 outShadowCoord;
void main()
{
	const mat4 biasMat = mat4( 
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5 - objectMaterial.shadowBias, 1.0
    );
	vec3 T = normalize(vec3(objectMatrix * vec4(inTangent, 0.0)));
	vec3 B = normalize(vec3(objectMatrix * vec4(inBitangent, 0.0)));
	vec3 N = normalize(vec3(objectMatrix * vec4(inNormal, 0.0)));
	mat3 TBN = mat3(T, B, N);
	vec4 worldSpace = objectMatrix * vec4(inPosition, 1);
    outPosition = worldSpace.xyz;
	outNormal = normalize(TBN * inNormal);
    outShadowCoord = (biasMat * light0.viewProj * worldSpace);
    gl_Position = projectionMatrix * viewMatrix * worldSpace;
}