#version 430 core

layout (location = 0) in vec3 lPos;
layout (location = 3) in vec2 lTexC;

out vec2 texC;

void main()
{
    texC = lTexC;
    gl_Position = vec4(lPos, 1.0);
}