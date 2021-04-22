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
struct AxisAlignedBoundingBox
{
	glm::vec3 bbMin = std::numeric_limits<glm::vec3>::max();
	glm::vec3 bbMax = std::numeric_limits<glm::vec3>::min();

	glm::vec3 center;
	glm::vec3 entents;
};

struct MeshGeometry
{
	MeshGeometry() {};
	MeshGeometry(std::string name) : name(name) {};

	std::string name;

	unsigned int mIndexCount = 0;
	// GPU resource: vertex array
	unsigned int vao;
	// CPU resource: vertex array
	std::vector<Vertex> mVertices;
	std::vector<unsigned int> mIndices;

	AxisAlignedBoundingBox bounds;

	void BuildResources();
	void BuildResources(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
	void BuildBoundingBox();
	void Draw(Shader* shader);
	void DrawLines(Shader* shader);
};

// Data transmitted by mesh loader
struct ModelData
{
	std::vector<Vertex> mVertices;
	std::vector<unsigned int> mIndices;
};

struct ModelMesh
{
	std::string name;
	std::vector<MeshGeometry> subMeshes;

	glm::vec3 position;

	AxisAlignedBoundingBox bounds;

	void BuildResources(std::vector<ModelData> modelData);
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

	glm::vec3 albedo = glm::vec3(0.1);
	float metallic = 0.1;
	float roughness = 0.1;
	float ambientOcclusion = 1.f;
	glm::vec3 emmisive = glm::vec3(0.0, 0.0, 0.0);

	bool useDiffuse = false;
	bool useNormal = false;

	unsigned int diffuseID = -1;
	unsigned int normalID = -1;
	// 3D texture
	unsigned int densityID = -1;
};

struct Texture
{
	std::string name;
	std::string path;
	std::vector<std::string> cubePath;

	unsigned int textureID;
	void BuildResource(TinyddsLoader::DDSFile& ddsFile);
	void BuildResource(std::string fileName);
	void BuildResource(std::vector<std::string> cubeFilenames);
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

	std::string name = "";

	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::mat4 world = glm::mat4(1.f);

	MeshGeometry* geo = nullptr;
	Material* mat = nullptr;

	int textureScale = 1;

	bool bCulled = false;

	void UpdateWorld();

	void Draw(Shader* shader);
	void DrawLines(Shader* shader);
};

struct Smoke
{
	Smoke(int x, int y, int z) : nx(x), ny(y), nz(z) {};

	std::string name;
	unsigned int densityFieldID = -1;
	float* density;
	
	int nx = 0;
	int ny = 0;
	int nz = 0;

	float decay = 0.05f;
	glm::vec3 force = glm::vec3(0, 0, 0);

	void BuildResource();
	~Smoke();
};