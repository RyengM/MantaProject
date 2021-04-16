#version 430 core

out vec4 FragColor;

in VS_OUT
{
    vec3 fragWorldPos;
	vec3 fragWorldNormal;
	vec2 fragTexC;
	vec3 T;
    vec3 B;
    vec3 N;
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
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
	bool useDiffuseMap;
	bool useNormalMap;
};

uniform PassCb passCb;
uniform Light light[2];
uniform Material material;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform int textureScale;

#define PI 3.141592653589793
#define PI2 6.283185307179586
#define PBR true

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 BlinnPhong(vec3 lightStrength, vec3 toLight, vec3 normal, vec3 toEye)
{
	float shininess = (1.0 - material.roughness) * 255;
    // Diffuse shading
    float diff = max(dot(normal, toLight), 0.0);
    // Specular shading
	vec3 halfVec = (normalize(toLight) + normalize(toEye)) / 2.0;
    float spec = pow(max(dot(normal, halfVec), 0.0), shininess);
    // Combine results
	vec3 diffuse = vec3(0.0);
	if (material.useDiffuseMap) diffuse = diff * lightStrength * material.albedo.xyz * texture(diffuseMap, ps_in.fragTexC * textureScale).xyz;
	else diffuse = diff * lightStrength * material.albedo.xyz;
    vec3 specular = spec * lightStrength * material.albedo.xyz;
    return passCb.ambientLight.xyz + diffuse + specular;
}

vec3 Cook_Torrance(Light light, vec3 L, vec3 N, vec3 V)
{
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, material.albedo, material.metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	// calculate per-light radiance
	vec3 H = normalize(V + L);
	float distance = length(light.pos - ps_in.fragWorldPos);
	float attenuation = 1.0 / (distance * distance);
	// vec3 radiance = light.strength * attenuation;
	vec3 radiance = light.strength;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, material.roughness);   
	float G   = GeometrySmith(N, V, L, material.roughness);      
	vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
	
	vec3 nominator    = NDF * G * F; 
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = nominator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0
	
	// kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - material.metallic;	  

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);        

	// add to outgoing radiance Lo
	if (material.useDiffuseMap) Lo = (kD * material.albedo * texture(diffuseMap, ps_in.fragTexC * textureScale).xyz / PI + specular) * radiance * NdotL;
	else Lo = (kD * material.albedo / PI + specular) * radiance * NdotL;

	// ambient lighting (note that the next IBL tutorial will replace 
	// this ambient lighting with environment lighting).
	vec3 ambient = vec3(0.03) * material.albedo * material.ao;

	vec3 color = ambient + Lo;

	return color;
}

// Calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal)
{
    vec3 L = normalize(-light.dir);
	vec3 V = normalize(passCb.eyePos - ps_in.fragWorldPos);
	vec3 N = normal;

	if (PBR)
		return Cook_Torrance(light, L, N, V);

    return BlinnPhong(light.strength, L, normal, V);
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

// ---Shadow-------------------------------
#define NUM_SAMPLES 40
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10
#define SHADOW_HARDNESS 800

#define EPS 1e-3

vec2 poissonDisk[NUM_SAMPLES];

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

highp float rand_2to1(vec2 uv ) { 
    // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

float useShadowMap(sampler2D shadowMap, vec4 shadowCoord){
  	vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
  	float closestDepth = unpack(texture2D(shadowMap, coord.xy));
	// float closestDepth = texture2D(shadowMap, coord.xy).r;
  	return closestDepth + EPS > coord.z ? 1.0 : 0.0;
}

float showLightDepth(sampler2D shadowMap, vec4 shadowCoord)
{
    vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
    float closestDepth = unpack(texture2D(shadowMap, coord.xy));
	// float closestDepth = texture2D(shadowMap, coord.xy).r;
    return closestDepth;
}

float PCF(sampler2D shadowMap, vec4 shadowCoord) {
	vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
	poissonDiskSamples(coord.xy);
	float visibility = 0.0;
	for (int i = 0; i < PCF_NUM_SAMPLES; i++) {
		float closestDepth = unpack(texture2D(shadowMap, coord.xy + poissonDisk[i] / SHADOW_HARDNESS));
		if (closestDepth + EPS > coord.z || closestDepth == 0.0) visibility += 1.0;
	}
	return visibility /= float(PCF_NUM_SAMPLES);
}

void main()
{
	vec3 color = vec3(0.f);
	vec3 normal = vec3(0.f);
	for (int i = 0; i < passCb.lightNum; ++i)
	{
		if (material.useNormalMap)
		{
			normal = texture(normalMap, ps_in.fragTexC * textureScale).rgb;
			normal = normalize(normal * 2.0 - 1.0);
			mat3 TBN = mat3(normalize(ps_in.T), normalize(ps_in.B), normalize(ps_in.N));
			normal = normalize(TBN * normal);
		}
		else 
			normal = normalize(ps_in.fragWorldNormal);
		if (light[i].type == 0) color += CalcDirLight(light[i], normal);
		else if (light[i].type == 1) color += CalcPointLight(light[i], normal);
	}
	float visibility;
	visibility = PCF(shadowMap, ps_in.shadowCoord);
	float depth = showLightDepth(shadowMap, ps_in.shadowCoord);

	// HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

	FragColor = vec4(vec3(color * visibility), 1.f);
}