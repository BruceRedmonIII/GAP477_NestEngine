#version 450
#extension GL_EXT_debug_printf : require
const uint eMaterialLit = 0x01 << 0;
const uint eMaterialCastShadows = 0x01 << 1;
const uint eMaterialCatchShadows = 0x01 << 2;
const uint eDiffuse = 0x01 << 0;
const uint eNormal = 0x01 << 1;
const uint eRoughness = 0x01 << 2;
const uint eMetallic = 0x01 << 3;
const uint eAmbient = 0x01 << 4;
const uint eHeight = 0x01 << 5;

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

layout(set = 1, binding = 1) uniform Lights
{
	Light lights[1];
	int lightCount;
};

layout(set = 2, binding = 2) uniform sampler2D shadowMap;

layout(location = 0) smooth in vec3 inPosition;
layout(location = 1) smooth in vec3 inNormal;
layout(location = 2) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;


float calculateAttenuation(float dist, Light light)
{
    return 1.0 / (light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist);
}

vec3 lighting(vec3 lightPos, vec3 pos, vec3 N, vec3 color)
{
    vec3 L = normalize(lightPos - pos);
    float NdotL = max(dot(N, L), 0);

    float lightDist = length(lightPos - pos);
    float atten = 1; //attenuation(lightDist, lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);

    return color * NdotL * atten;
}

vec3 blinnPhong(Light light, vec4 color, vec3 surfacePosition, vec3 N)
{
	// Light direction and distance
	vec3 L = normalize(light.position - surfacePosition);
	float lightDist = length(light.position - surfacePosition);
	float lightAtten = calculateAttenuation(lightDist, light);
	
	// View direction and half direction
	vec3 V = normalize(cameraPosition - surfacePosition);
	vec3 H = normalize(L + V);
	
    // Diffuse component
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = color.rgb * light.diffuse.rgb * NdotL * lightAtten;
    
    // Specular component
    float NdotH = max(dot(N, H), 0.0);
    vec3 specular = light.diffuse.rgb * pow(NdotH, objectMaterial.shininess) * lightAtten;
	
	// return
	return diffuse + specular;
}

const float ShadowAmbient = 0.1;

float lookupShadow(vec4 shadowCoord)
{
    vec4 shadowPos = shadowCoord / shadowCoord.w;
    if (shadowPos.z > -1.0 && shadowPos.z < 1.0)
    {
		float shadowDepth = texture(shadowMap, shadowPos.st).r;
        if (shadowPos.w > 0.0 && shadowDepth < shadowPos.z)
        {
            return ShadowAmbient;
        }
    }
	return 1.0;
}

vec3 diffuse(vec3 surfacePosition, vec3 surfaceNormal, Light light)
{
    vec3 L = normalize(light.position - surfacePosition);
    vec3 N = normalize(surfaceNormal);
    float NdotL = max(dot(N, L), 0.0);
    return light.diffuse * NdotL;
}

vec3 calculateLighting()
{
    vec3 totalLight = vec3(1.0);
	for (int i = 0; i < lightCount; ++i)
	{
		totalLight += diffuse(inPosition, inNormal, lights[i]);
	}
    return totalLight;
}

void main()
{
    float shadow = 1.0;
    if (!objectMaterial.enableLighting) 
	{
        outColor = objectMaterial.color;
        return;
    }
	vec3 totalLight = vec3(1.0);
	vec4 color = objectMaterial.color;
	//shadows
	if ((objectMaterial.options & eMaterialCatchShadows) != 0)
        shadow = lookupShadow(inShadowCoord);

    if ((objectMaterial.options & eMaterialLit) != 0)
        totalLight = calculateLighting();
	
    outColor = vec4(color.rgb * totalLight * shadow, color.a);
}