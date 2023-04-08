#pragma once

#include "ShaderHandler.h"
#include "Mesh.h"

#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>
#include "stb_image.h"
#include <iostream>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
unsigned int LoadCubeMapFromFile(std::vector<std::string> faces, std::string textureDir, bool gamma = false);

enum DEFAULT_MODELS {
    PLANE = 0,
    CUBE,
    SPHERE,
    QUAD
};

class Model
{
public:
    Model(DEFAULT_MODELS modelType, std::string modelName) 
    {
        std::string modelPath = GetCurrentDir() + "\\Models\\";
        switch (modelType) {
            case DEFAULT_MODELS::PLANE :
                modelPath += "plane\\plane.obj";
                break;
            case DEFAULT_MODELS::CUBE :
                modelPath += "cube\\cube.obj";
                break;
            case DEFAULT_MODELS::SPHERE :
                modelPath += "sphere\\sphere.obj";
                break;
            case DEFAULT_MODELS::QUAD : 
                modelPath += "plane\\quad.obj";
                break;
        }

        loadModel(modelName, modelPath);
    }

    Model(std::string modelName, std::string path)
    {
        loadModel(modelName, path);
    }

    std::string GetModelName() { return modelName; }

    void Draw(std::shared_ptr<Shader> shader);
private:
    // model data
    std::string modelName;
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    void loadModel(std::string modelName, std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};