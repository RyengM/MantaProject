#version 430 core

layout (location = 0) in vec3 lPos;

out vec3 texCoord;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    texCoord = lPos;
    // set z to 1.0 so that it can always fail the depth test
    gl_Position = (proj * view * vec4(lPos, 1.0)).xyww;
}