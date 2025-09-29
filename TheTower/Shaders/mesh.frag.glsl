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
const uint ePoint = 0;
const uint eSun = 1;
const uint eSpot = 2;
const uint eArea = 3;
const float PI = 3.14159265359;
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
	vec3 specular;
	vec3 ambient;
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


float calculateAttenuation(float dist, vec3 atten)
{
    return 1.0 / (atten.x + atten.y * dist + atten.z * dist * dist);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
vec3 PBRLightCalculation(Light light, vec3 albedo, float metallic, float roughness, vec3 normal)
{
	//---------------------------------------------
	// Gather viewPosition
	vec3 V = normalize(cameraPosition - inPosition);
	
	// This is a reflectance equation
	// We start with determining F0 which stands for
	// surface reflection at zero incidence,
	// aka, how much the surface reflects if looking directly at the surface
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
	vec3 L = normalize(light.position - inPosition);
	// Halfway position used to calculate light reflection
	vec3 H = normalize(V + L);
	float dist = length(light.position - inPosition);
	float attenuation = calculateAttenuation(dist, light.attenuation);
	vec3 radiance = light.diffuse * attenuation;
	
	// Cook-Torrance BRDF
	float NDF = DistributionGGX(normal, H, roughness);
    float G   = GeometrySmith(normal, V, L, roughness);      
	// The fresnel approximation is used to determine how much a surface reflects light
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
	vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(normal, L), 0.0);        

    // add to outgoing radiance Lo
	// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again  
    return (kD * albedo / PI + specular) * radiance * NdotL;
}
vec3 SpotLight(Light light, vec3 albedo, float metallic, float roughness, vec3 normal)
{
	vec3 lightDir = normalize(-light.direction);
	vec3 lightToPixel = normalize(inPosition - light.position);
	float spotFactor = dot(lightToPixel, lightDir);
	if (spotFactor > light.cutoff)
	{
		vec3 color = PBRLightCalculation(light, albedo, metallic, roughness, normal);
		float spotLightIntensity = (1.0 - (1.0 - spotFactor)/1.0 - light.cutoff);
		return color * spotLightIntensity;
	}
	return vec3(0.0);
}

vec4 CalculateLighting()
{
	vec4 albedo = objectMaterial.color;
	vec3 normal = inNormal;
	float metallic = objectMaterial.metallic;
	float roughness = objectMaterial.roughness;
	vec3 result = vec3(0.0);
	for (int i = 0; i < lightCount; ++i)
	{
		uint type = lights[i].type; 
		if (type == eSpot)
		{   // if were using a spotlight, we run a check to make sure the fragment is within the spotlights range/cone
			result += SpotLight(lights[i], albedo.rgb, metallic, roughness, normal);
		}
		else
		{
			result += PBRLightCalculation(lights[i], albedo.rgb, metallic, roughness, normal);
		}
	}
	// Add default ambient light to avoid unlit areas from being entirely black
	vec3 ambient = vec3(0.03) * albedo.rgb;
    vec3 color = ambient + result;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
	return vec4(color * albedo.rgb, albedo.a);
}
void main()
{
    if (!objectMaterial.enableLighting) 
	{
        outColor = objectMaterial.color;
        return;
    }

    float shadow = 1.0;

	//shadows
	if ((objectMaterial.options & eMaterialCatchShadows) != 0)
        shadow = lookupShadow(inShadowCoord);
	
	vec4 colorResult = vec4(0.0);
    if ((objectMaterial.options & eMaterialLit) != 0)
        colorResult = CalculateLighting();
    outColor = vec4(colorResult.rgb * shadow, colorResult.a);
}