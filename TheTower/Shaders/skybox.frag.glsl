#version 450

layout(binding = 0) uniform Camera
{
    mat4 projectionMatrix;
    mat4 viewMatrix;

    vec3 lightPosition;
    vec3 lightColor;
    vec3 lightAttenuation;
};

layout(set = 1, binding = 3) uniform samplerCube skybox;

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(skybox, inUVW);
}
