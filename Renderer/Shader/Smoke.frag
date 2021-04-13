#version 430 core

in VS_OUT
{
    vec3 fragWorldPos;
    vec3 bbMinWorld;
    vec3 bbMaxWorld;
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
	vec3 pos;
	vec3 strength;
	vec3 dir;
};

struct Ray
{
	vec3 pos;
	vec3 dir;
};

Ray CreateRay(vec3 pos, vec3 dir)
{
	Ray ray;
	ray.pos = pos;
	ray.dir = dir;
	return ray;
}

uniform PassCb passCb;
uniform Light light;

uniform sampler3D densityTexture;

#define E 2.718281828459

void main()
{
	Ray ray = CreateRay(passCb.eyePos, normalize(ps_in.fragWorldPos - passCb.eyePos));

	// ray intersection
	vec3 firstIntersection = (ps_in.bbMinWorld - passCb.eyePos) / ray.dir;
	vec3 secondIntersection = (ps_in.bbMaxWorld - passCb.eyePos) / ray.dir;
	vec3 closest = min(firstIntersection, secondIntersection);
	vec3 furthest = max(firstIntersection, secondIntersection);

	// the distance between camera and hit point of the nearst and furthest plane
	float t0 = max(closest.x, max(closest.y, closest.z));
	float t1 = min(furthest.x, min(furthest.y, furthest.z));

	t0 = max(0, t0);

	// the distance between entry point and out point
	float boxThickness = max(0, t1 - t0);
	vec3 entryPos = passCb.eyePos + t0 * ray.dir;

	float shadowSteps = 64.f;
	float raymarchStep = 256.f;
    vec3 bbOppsite = ps_in.bbMaxWorld - ps_in.bbMinWorld;
	// ensure ray can walk through the volume
	int maxSteps = int(raymarchStep * length(bbOppsite)) + 1;

    // assume the smoke is influenced by main light only
	vec3 lightVec = -normalize(light.dir);
	lightVec *= 1.f / shadowSteps;
	vec3 localCamVec = ray.dir / raymarchStep / bbOppsite;

	// convert the sample range to 0-1
	vec3 curPos = (entryPos - (ps_in.bbMinWorld + ps_in.bbMaxWorld - bbOppsite) * 0.5f) / bbOppsite;
	vec3 lightE = vec3(0.584f, 0.514f, 0.451f);
	float lightEnergy = 0.f;
	float transmittance = 1.f;
	float alpha = 0.f;

	for (int i = 0; i < maxSteps; ++i)
	{	
		float cursample = texture(densityTexture, curPos).x;

		if(cursample > 0.001f)
		{
			vec3 lightPos = curPos;
			float shadow = 0.f;

			for (int s = 0; s < shadowSteps; ++s)
			{
				lightPos += lightVec;
				float lightSample = texture(densityTexture, lightPos).x;

				if (lightSample < 0.005f)
					continue;

				shadow += lightSample;
			}

			float curdensity = clamp(cursample / 64.f, 0, 1);
			float shadowterm = exp(-shadow * 0.01f);
			float absorbedlight = shadowterm * curdensity;
			lightEnergy += absorbedlight * transmittance;
			transmittance *= 1.f - curdensity;
		}
    
		curPos += localCamVec;
	}

    gl_FragColor = vec4((lightEnergy + 0.8f) * 0.5f * lightE, lightEnergy * 1.5f);
}