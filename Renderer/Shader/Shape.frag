#version 430 core
out vec4 FragColor;

in VS_OUT
{
    vec3 fragWorldPos;
	vec3 fragWorldNormal;
	vec3 fragWorldTangent;
	vec2 fragTexC;
	mat3 TBN;
	vec4 shadowCoord;
} ps_in;

struct PassCb
{
    mat4 view;
    mat4 proj;
	vec3 eyePos;
	vec4 ambientLight;
	int lightNum;
};

struct Light
{
	int type;
	vec3 pos;
	vec3 strength;
	vec3 dir;
	float fallStart;
	float fallEnd;
	float spotPower;
};

struct Material
{
	vec4 diffuse;
	vec3 fresnelR0;
	float roughness;
};

uniform PassCb passCb;
uniform Light light[2];
uniform Material material;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform int textureScale;

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

vec3 BlinnPhong(vec3 lightStrength, vec3 toLight, vec3 normal, vec3 toEye)
{
	float shininess = (1.0 - material.roughness) * 255;
    // Diffuse shading
    float diff = max(dot(normal, toLight), 0.0);
    // Specular shading
	vec3 halfVec = (normalize(toLight) + normalize(toEye)) / 2.f;
    float spec = pow(max(dot(normal, halfVec), 0.0), shininess);
    // Combine results
    vec3 diffuse = diff * lightStrength * material.diffuse.xyz * texture(diffuseMap, ps_in.fragTexC * textureScale).xyz;
    vec3 specular = spec * lightStrength * material.diffuse.xyz;
    return passCb.ambientLight.xyz + diffuse + specular;
}

// Calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal)
{
    vec3 lightDir = normalize(-light.dir);
	vec3 eyeDir = normalize(passCb.eyePos - ps_in.fragWorldPos);
    return BlinnPhong(light.strength, lightDir, normal, eyeDir);
}

// Calculates the color when using a point light.
vec3 CalcPointLight(Light light, vec3 normal)
{
    vec3 lightDir = normalize(light.pos - ps_in.fragWorldPos);
    vec3 eyeDir = normalize(passCb.eyePos - ps_in.fragWorldPos);
	float lightLen = length(light.pos - ps_in.fragWorldPos);
	vec3 strength = light.strength / pow(lightLen, 2);
	return BlinnPhong(strength, lightDir, normal, eyeDir);
}

vec3 NormalSampleToWorldSpace(vec3 normalMapSample, vec3 unitNormalW, vec3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	vec3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	vec3 N = unitNormalW;
	vec3 T = normalize(tangentW - dot(tangentW, N)*N);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	// Transform from tangent space to world space.
	vec3 bumpedNormalW = normalT * TBN;

	return bumpedNormalW;
}

float useShadowMap(sampler2D shadowMap, vec4 shadowCoord){
  vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
  float closestDepth = unpack(texture2D(shadowMap, coord.xy));
//   float closestDepth = texture2D(shadowMap, coord.xy).r;
  return closestDepth + 0.0005 > coord.z ? 1.0 : 0.0;
}

float showLightDepth(sampler2D shadowMap, vec4 shadowCoord)
{
    vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
    // float closestDepth = unpack(texture2D(shadowMap, coord.xy));
	float closestDepth = texture2D(shadowMap, coord.xy).r;
    return closestDepth;
}

void main()
{
	vec3 color = vec3(0.f);
	vec3 normal = vec3(0.f);
	for (int i = 0; i < passCb.lightNum; ++i)
	{
		// normal = texture(normalMap, ps_in.fragTexC * textureScale).xyz;
		// normal = normalize(normal * 2.0 - 1.0);
		// normal = normalize(ps_in.TBN * normal);
		// normal = NormalSampleToWorldSpace(normal, ps_in.fragWorldNormal, ps_in.fragWorldTangent);
		if (light[i].type == 0) color += CalcDirLight(light[i], normalize(ps_in.fragWorldNormal));
		else if (light[i].type == 1) color += CalcPointLight(light[i], normal);
	}
	float visibility;
	visibility = useShadowMap(shadowMap, ps_in.shadowCoord);
	float depth = showLightDepth(shadowMap, ps_in.shadowCoord);
	// FragColor = vec4(depth);
	FragColor = vec4(vec3(color * visibility), 1.f);
}