#include "MeshLoader.h"
#include <iostream>

std::vector<ModelData> MeshLoader::LoadModel(std::string path)
{
    std::vector<ModelData> modelData;

    const aiScene* scene = meshImporter.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << meshImporter.GetErrorString() << std::endl;
        return modelData;
    }

    ProcessNode(scene->mRootNode, scene, modelData);

    return modelData;
}

void MeshLoader::ProcessNode(aiNode* node, const aiScene* scene, std::vector<ModelData>& modelData)
{
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        modelData.push_back(std::move(ProcessMesh(mesh, scene)));
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, modelData);
    }
}

ModelData MeshLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    ModelData data;
    Vertex v;
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        v.pos.x = mesh->mVertices[i].x;
        v.pos.y = mesh->mVertices[i].y;
        v.pos.z = mesh->mVertices[i].z;

        v.normal.x = mesh->mNormals[i].x;
        v.normal.y = mesh->mNormals[i].y;
        v.normal.z = mesh->mNormals[i].z;

        if (mesh->mTangents)
        {
            v.tangentU.x = mesh->mTangents[i].x;
            v.tangentU.y = mesh->mTangents[i].y;
            v.tangentU.z = mesh->mTangents[i].z;
        }

        if (mesh->mTextureCoords[0])
        {
            v.texC.x = mesh->mTextureCoords[0][i].x;
            v.texC.y = mesh->mTextureCoords[0][i].y;
        }
        else
            v.texC = glm::vec2(0.f);

        data.mVertices.push_back(v);
    }
       
    for (auto i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (auto j = 0; j < mesh->mFaces->mNumIndices; j++)
            data.mIndices.push_back(face.mIndices[j]);
    }

    return data;
}