#include "Utils.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
}

void MeshGeometry::Draw(Shader* shader)
{
	glBindVertexArray(vao);
	//glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, &mIndices[0]);
	glBindVertexArray(0);
}

void Texture::BuildResource(TinyddsLoader::DDSFile& ddsFile)
{
	glGenTextures(1, &textureID);

	if (!TinyddsLoader::OpenGLDSS::LoadGLTexture(textureID, ddsFile))
		std::cout << "Texture failed to load at path: " << path << std::endl;
}