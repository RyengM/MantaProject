#pragma once

#include <vector>
#include <string>

class TextureLoader
{
public:
	TextureLoader() {};

	static void LoadTexture(unsigned int& textureID, const std::string filePath);

	static void LoadCubemap(unsigned int& textureID, const std::vector<std::string> faces);
};