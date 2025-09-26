#version 450
layout(push_constant) uniform Constants
{
    mat4 light0Matrix;
    mat4 objectMatrix;
};

layout(location = 0) in vec4 inPosition;

void main()
{	
    gl_Position = light0Matrix * objectMatrix * inPosition;
}