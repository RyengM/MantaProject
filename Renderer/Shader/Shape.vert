#version 430 core

layout (location = 0) in vec3 lPos;
layout (location = 1) in vec3 lNormal;
layout (location = 2) in vec3 lTangentU;
layout (location = 3) in vec2 lTexC;

out VS_OUT
{
    vec3 fragWorldPos;
    vec3 fragWorldNormal;
	vec3 fragWorldTangent;
    vec2 fragTexC;
    mat3 TBN;
    vec4 shadowCoord;
} vs_out;

uniform mat4 model;

struct PassCb
{
    mat4 view;
    mat4 proj;
    vec3 eyePos;
    vec4 ambientLight;
    int lightNum;
};
uniform PassCb passCb;

uniform mat4 lightProjView;

void main()
{
	gl_Position = passCb.proj * passCb.view * model * vec4(lPos, 1.f);
    vec3 T = normalize(vec3(model * vec4(lTangentU, 0.f)));
    vec3 N = normalize(vec3(model * vec4(lNormal, 0.f)));
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);
	vs_out.fragWorldPos = vec3(model * vec4(lPos, 1.f));
    vs_out.fragWorldNormal = vec3(vec4(lNormal, 1.f) * model);
    vs_out.fragWorldTangent = vec3(vec4(lTangentU, 1.f) * model);
    vs_out.fragTexC = lTexC;
    vs_out.TBN = mat3(T, B, N);
    vs_out.shadowCoord = lightProjView * model * vec4(lPos, 1.f);
}