#include "Utils.h"
#include "TextureLoader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void MeshGeometry::BuildResources()
{
	// Build GPU resource
	glGenVertexArrays(1, &vao);
	unsigned int vbo, ebo;
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::tangentU));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::texC));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	mIndexCount = mIndices.size();

	BuildBoundingBox();
}

void MeshGeometry::BuildResources(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
	// Build GPU resource
	glGenVertexArrays(1, &vao);
	unsigned int vbo, ebo;
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::tangentU));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::texC));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Build CPU resource
	mVertices = std::move(vertices);
	mIndices = std::move(indices);

	mIndexCount = mIndices.size();

	BuildBoundingBox();
}

void MeshGeometry::BuildBoundingBox()
{
	for (int i = 0; i < mVertices.size(); ++i)
	{
		glm::vec3 v = mVertices[i].pos;
		bounds.bbMin.x = bounds.bbMin.x < v.x ? bounds.bbMin.x : v.x;
		bounds.bbMin.y = bounds.bbMin.y < v.y ? bounds.bbMin.y : v.y;
		bounds.bbMin.z = bounds.bbMin.z < v.z ? bounds.bbMin.z : v.z;
		bounds.bbMax.x = bounds.bbMax.x > v.x ? bounds.bbMax.x : v.x;
		bounds.bbMax.y = bounds.bbMax.y > v.y ? bounds.bbMax.y : v.y;
		bounds.bbMax.z = bounds.bbMax.z > v.z ? bounds.bbMax.z : v.z;
	}

	bounds.center = (bounds.bbMax + bounds.bbMin) / 2.f;
	bounds.entents = (bounds.bbMax - bounds.bbMin) / 2.f;
}

void MeshGeometry::Draw(Shader* shader)
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, &mIndices[0]);
	glBindVertexArray(0);
}

void MeshGeometry::DrawLines(Shader* shader)
{
	glBindVertexArray(vao);
	glDrawElements(GL_LINES, mIndexCount, GL_UNSIGNED_INT, &mIndices[0]);
	glBindVertexArray(0);
}

void ModelMesh::BuildResources(std::vector<ModelData> modelData)
{
	for (auto data : modelData)
	{
		MeshGeometry bunnyGeo;
		bunnyGeo.mVertices = std::move(data.mVertices);
		bunnyGeo.mIndices = std::move(data.mIndices);
		bunnyGeo.BuildResources();
		subMeshes.push_back(std::move(bunnyGeo));
	}
}

void Texture::BuildResource(TinyddsLoader::DDSFile& ddsFile)
{
	glGenTextures(1, &textureID);

	if (!TinyddsLoader::OpenGLDSS::LoadGLTexture(textureID, ddsFile))
		std::cout << "Texture failed to load at path: " << path << std::endl;
}

void Texture::BuildResource(std::string fileName)
{
	glGenTextures(1, &textureID);

	TextureLoader::LoadTexture(textureID, fileName);
}

void Texture::BuildResource(std::vector<std::string> cubeFilenames)
{
	glGenTextures(1, &textureID);

	TextureLoader::LoadCubemap(textureID, cubeFilenames);
}

void RenderItem::UpdateWorld()
{
	world = glm::scale(glm::translate(glm::mat4(1.f), position), scale);
}

void RenderItem::Draw(Shader* shader)
{
	geo->Draw(shader);
}

void RenderItem::DrawLines(Shader* shader)
{
	geo->DrawLines(shader);
}

void Smoke::BuildResource()
{
	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &densityFieldID);
	glBindTexture(GL_TEXTURE_3D, densityFieldID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_3D, 0);

	density = (float*)malloc(nx * ny * nz* sizeof(float));
	memset(density, 0, nx * ny * nz * sizeof(float));
}

Smoke::~Smoke()
{
	free(density);
}