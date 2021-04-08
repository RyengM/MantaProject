#version 430 core

layout (location = 0) in vec3 lPos;

uniform mat4 lightProjView;
uniform mat4 model;

void main()
{
	gl_Position = lightProjView * model * vec4(lPos, 1.f);
}