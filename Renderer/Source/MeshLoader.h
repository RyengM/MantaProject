#pragma once

#include "Utils.h"
#include "Assimp/Importer.hpp"
#include "Assimp/scene.h"
#include "Assimp/postprocess.h"
#include <string>

class MeshLoader
{
public:
	MeshLoader() {};

	std::vector<ModelData> LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene, std::vector<ModelData>& modelData);
	ModelData ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
	Assimp::Importer meshImporter;
};