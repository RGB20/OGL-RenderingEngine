#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include "Utilities.h"
#include "ShaderHandler.h"

#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>
#include "stb_image.h"
#include <iostream>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
unsigned int LoadCubeMapFromFile(std::vector<std::string> faces, std::string textureDir, bool gamma = false);

enum DEFAULT_MESHES {
    PLANE = 0,
    CUBE,
    SPHERE,
    QUAD
};

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;  // we store the path of the texture to compare with other textures
};

class Mesh {
public:
    Mesh(DEFAULT_MESHES meshType, std::string meshName)
    {
        std::string meshPath = GetCurrentDir() + "\\Models\\";
        switch (meshType) {
        case DEFAULT_MESHES::PLANE:
            meshPath += "plane\\plane.obj";
            break;
        case DEFAULT_MESHES::CUBE:
            meshPath += "cube\\cube.obj";
            break;
        case DEFAULT_MESHES::SPHERE:
            meshPath += "sphere\\sphere.obj";
            break;
        case DEFAULT_MESHES::QUAD:
            meshPath += "plane\\quad.obj";
            break;
        }

        LoadMesh(meshName, meshPath);
    }

    Mesh(std::string meshName, std::string meshPath)
    {
        LoadMesh(meshName, meshPath);
    }

    void Draw(std::shared_ptr<Shader> shader, bool instancing, uint32_t instancingCount);

    unsigned int GetMeshVAO() { return VAO; }

private:
    void LoadMesh(std::string meshName, std::string meshPath);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

    // Mesh directory
    std::string directory;
    
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    std::vector<Texture> textures_loaded;

    //  render data
    unsigned int VAO, VBO, EBO;

    void SetupMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
};