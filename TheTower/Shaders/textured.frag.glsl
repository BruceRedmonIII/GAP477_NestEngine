#version 450
#extension GL_EXT_debug_printf : require
const uint eMaterialLit = 0x01 << 0;
const uint eMaterialCastShadows = 0x01 << 1;
const uint eMaterialCatchShadows = 0x01 << 2;
const uint eDiffuse = 0x01 << 0;
const uint eNormal = 0x01 << 1;
const uint eRoughness = 0x01 << 2;
const uint eMetallic = 0x01 << 3;
const uint eAO = 0x01 << 4;
const uint eOpacity = 0x01 << 5;
const uint ePoint = 0;
const uint eSun = 1;
const float PI = 3.14159265359;
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
layout(set = 3, binding = 3) uniform sampler2D diffuseTexture;
layout(set = 4, binding = 4) uniform sampler2D normalTexture;
layout(set = 5, binding = 5) uniform sampler2D roughnessTexture;
layout(set = 6, binding = 6) uniform sampler2D metallicTexture;
layout(set = 7, binding = 7) uniform sampler2D ambientOcclusionTexture;
layout(set = 8, binding = 8) uniform sampler2D opacityTexture;

layout(location = 0) smooth in vec3 inPosition;
layout(location = 1) smooth in vec3 inNormal;
layout(location = 2) in vec4 inShadowCoord;
layout(location = 3) in vec2 inTexcoord0;
layout(location = 4) smooth in vec3 inTestNormal;

layout(location = 0) out vec4 outColor;

float calculateAttenuation(float dist, Light light)
{
    return 1.0 / (light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * (dist * dist));
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
float SampleMetallic()
{
	float metallic = 0.0;
	if ((objectMaterial.textureFlags & eMetallic) != 0)
	{
		metallic = texture(metallicTexture, inTexcoord0).r;
	}
	return metallic;
}
float SampleOpacity()
{
	return texture(opacityTexture, inTexcoord0).r;
}
vec4 GetSurfaceColor()
{
	vec4 color = objectMaterial.color;
	if ((objectMaterial.textureFlags & eDiffuse) != 0)
	{
		color = texture(diffuseTexture, inTexcoord0);
	}
	return color;
}
vec3 GetNormal()
{
	vec3 normal = inNormal;
	if ((objectMaterial.textureFlags & eNormal) != 0)
	{
		normal = texture(normalTexture, inTexcoord0).xyz;
		// We have to map to a range of 0..1 to -1..+1
		normal = normalize(normal * 2.0 - 1.0);
		normal = (objectMatrix * vec4(normal, 0.0)).xyz;
	}
	return normal;
}

float SampleRoughness()
{
	float roughness = 0.0;
	if ((objectMaterial.textureFlags & eRoughness) != 0)
	{
		roughness = texture(roughnessTexture, inTexcoord0).r;
	}
	return roughness;
}
float SampleAmbientOcclusion()
{
	float ao = 1.0;
	if ((objectMaterial.textureFlags & eAO) != 0)
	{
		ao = texture(ambientOcclusionTexture, inTexcoord0).r;
	}
	return ao;
}

vec3 SunLighting(Light light, vec3 color, vec3 surfacePosition, vec3 normal)
{
  float sunStrength = .5;
  vec3 sunAmbient = sunStrength * light.diffuse; // color - grey
  vec3 lightDir = light.position;
  float roughness = SampleRoughness();

  float diffuseStrength = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diffuseStrength * light.diffuse;

  vec3 viewDir = normalize(cameraPosition - surfacePosition);
  vec3 reflectDir = normalize(reflect(-lightDir, normal));
  float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), roughness);
  vec3 specular = specularStrength * light.diffuse;

  vec3 lighting = sunAmbient + diffuse + specular;
  return color * lighting;
}
vec3 PointLighting(Light light, vec3 color, vec3 surfacePosition, vec3 normal)
{
    float roughness = SampleMetallic();
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	float diffuseStrength = max(dot(light.position, normal), 0.0);
	vec3 diffuse = diffuseStrength * light.diffuse;
	
	// V is view direction
	vec3 viewDir = normalize(cameraPosition);
	// light position needs to be reversed as the light is opposite of its projected light
	vec3 reflectDir = normalize(reflect(-light.position, normal));
	float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), roughness);
	vec3 specular = specularStrength * light.diffuse;
	
	vec3 lighting = ambient + diffuse + specular;
	return color * lighting;
}
vec3 calculateLighting(vec4 surfaceColor, vec3 normalPos)
{
	vec3 totalLight = vec3(1.0);
	for (int i = 0; i < lightCount; ++i)
	{
		if ((lights[i].type & eSun) != 0)
		{
			totalLight += SunLighting(lights[i], surfaceColor.rgb, inPosition, normalPos);
		}
		else
		{
			totalLight += PointLighting(lights[i], surfaceColor.rgb, inPosition, normalPos);
		}
	}
	return totalLight;
}

void main()
{
    float shadow = 1.0;
	float alpha = 1.0;
    if (!objectMaterial.enableLighting) 
	{
        outColor = objectMaterial.color;
        return;
    }

	vec4 surfaceColor = GetSurfaceColor();
	if ((objectMaterial.options & eOpacity) != 0)
		alpha = SampleOpacity();
	else
		alpha = surfaceColor.a;
	vec3 color = pow(texture(diffuseTexture, inTexcoord0).rgb, vec3(2.2));
	surfaceColor = vec4(color.rgb, surfaceColor.a);
	vec3 normal = GetNormal();

	vec3 totalLight = vec3(1.0);

	//shadows
	if ((objectMaterial.options & eMaterialCatchShadows) != 0)
        shadow = lookupShadow(inShadowCoord);

    if ((objectMaterial.options & eMaterialLit) != 0)
        totalLight = calculateLighting(surfaceColor, normal);
	
    outColor = vec4(surfaceColor.rgb * totalLight.rgb, alpha);
}