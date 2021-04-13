#pragma once

#include "GeometryGenerator.h"
#include "Shader.h"
#include "TinyddsLoader.h"
#include <unordered_map>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 tangentU;
	glm::vec2 texC;
};

// AABB
struct BoundingBox
{
	glm::vec3 bbMin = std::numeric_limits<glm::vec3>::max();
	glm::vec3 bbMax = std::numeric_limits<glm::vec3>::min();
};

struct MeshGeometry
{
	std::string name;

	unsigned int indexCount = 0;
	// GPU resource: vertex array
	unsigned int vao;
	// CPU resource: vertex array
	std::vector<Vertex> mVertices;
	std::vector<unsigned int> mIndices;

	BoundingBox bounds;

	void BuildResources(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
	void BuildBoundingBox();
	void Draw(Shader* shader);
};

struct Light
{
	// 0: Directional, 1: Point, 2: Spot
	int type = 0;
	// Common
	glm::vec3 strength = { 0.5f, 0.5f, 0.5f };
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	// Point and Spot
	float falloffStart = 1.0f;
	float falloffEnd = 10.0f;
	// Directional and Spot
	glm::vec3 focalPoint = { 0.0f, 0.0f, 0.0f };
	// Spot
	float spotPower = 64.0f;
};

struct Material
{
	std::string name;

	glm::vec4 diffuse = glm::vec4(0.1, 0.1, 0.1, 1.0);
	glm::vec3 fresnelR0 = glm::vec3(0.1, 0.1, 0.1);
	float roughness = 0.1;
	glm::vec3 emmisive = glm::vec3(0.0, 0.0, 0.0);

	unsigned int diffuseID = -1;
	unsigned int normalID = -1;
	unsigned int densityID = -1;
};

struct Texture
{
	std::string name;
	std::string path;

	unsigned int textureID;
	void BuildResource(TinyddsLoader::DDSFile& ddsFile);
};

struct PassCb
{
	glm::mat4 view = glm::mat4(1.f);
	glm::mat4 proj = glm::mat4(1.f);
	glm::vec3 eyePos = glm::vec3(1.f);
	glm::vec4 ambientLight = glm::vec4(glm::vec3(0.01), 1.f);
	int lightNum = 0;
};

struct RenderItem
{
	RenderItem() = default;
	RenderItem(const RenderItem& rhs) = delete;

	glm::mat4 world = glm::mat4(1.f);

	MeshGeometry* geo = nullptr;
	Material* mat = nullptr;

	int textureScale = 1;

	inline void Draw(Shader* shader)
	{
		geo->Draw(shader);
	}
};

struct Smoke
{
	std::string name;
	unsigned int densityFieldID = -1;
	float* density;

	void BuildResource();
	~Smoke();
};