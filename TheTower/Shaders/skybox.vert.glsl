#version 450

layout(binding = 0) uniform Camera
{
    mat4 projectionMatrix;
    mat4 viewMatrix;

    vec3 lightPosition;
    vec3 lightColor;
    vec3 lightAttenuation;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outUVW;

void main()
{
    outUVW = inPosition;
    gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) * vec4(inPosition, 1);
}