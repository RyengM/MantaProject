#version 430 core

layout (location = 0) in vec3 lPos;

out VS_OUT
{
    vec3 fragWorldPos;
    vec3 bbMinWorld;
    vec3 bbMaxWorld;
} vs_out;

struct PassCb
{
    mat4 view;
    mat4 proj;
    vec3 eyePos;
    vec4 ambientLight;
    int lightNum;
};
uniform PassCb passCb;

uniform mat4 model;
uniform vec3 bbMin;
uniform vec3 bbMax;

void main()
{
    gl_Position = passCb.proj * passCb.view * model * vec4(lPos, 1.f);
    vs_out.fragWorldPos = vec3(model * vec4(lPos, 1.f));
    vs_out.bbMinWorld = vec3(model * vec4(bbMin, 1.f));
    vs_out.bbMaxWorld = vec3(model * vec4(bbMax, 1.f));
}