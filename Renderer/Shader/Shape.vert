#version 430 core

layout (location = 0) in vec3 lPos;
layout (location = 1) in vec3 lNormal;
layout (location = 2) in vec3 lTangentU;
layout (location = 3) in vec2 lTexC;

out VS_OUT
{
    vec3 fragWorldPos;
    vec3 fragWorldNormal;
    vec2 fragTexC;
    vec3 T;
    vec3 B;
    vec3 N;
    vec4 shadowCoord;
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

uniform mat4 lightProjView;
uniform mat4 model;

void main()
{
	gl_Position = passCb.proj * passCb.view * model * vec4(lPos, 1.f);
    // we should insure that TBN is normalized again in fragment shader
    vs_out.T = normalize(vec3(model * vec4(lTangentU, 0.f)));
    vs_out.N = normalize(vec3(model * vec4(lNormal, 0.f)));
    // re-orthogonalize T with respect to N
    vs_out.T = normalize(vs_out.T - dot(vs_out.T, vs_out.N) * vs_out.N);
    vs_out.B = cross(vs_out.T, vs_out.N);
	vs_out.fragWorldPos = vec3(model * vec4(lPos, 1.f));
    vs_out.fragWorldNormal = vec3(vec4(lNormal, 1.f) * model);
    vs_out.fragTexC = lTexC;
    vs_out.shadowCoord = lightProjView * model * vec4(lPos, 1.f);
}