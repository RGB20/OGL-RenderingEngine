#pragma once
#include "Utilities.h"
#include "Camera.h"
#include "Mesh.h"
#include <unordered_map>

const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

class Scene {
public:
    Scene() {}

    void AddShader(std::string shaderName, std::unordered_map<SHADER_TYPES, std::string> shaders);// std::string vertexShaderPath, std::string fragmentShaderPath);
    void AddMesh(std::string modelName, std::string modelPath);
    void AddPresetMesh(std::string modelName, DEFAULT_MESHES modelType);
    void AddCamera(std::string cameraName, std::shared_ptr<Camera> camera);
    void LoadTexture(std::string textureName, std::string textureFileName, std::string textureDir);
    void LoadCubeMapTexture(std::string textureName, std::vector<std::string> faces, std::string textureDir);
    void AddMeshParameter(std::string parameterName, std::vector<glm::vec3> parameterArray);

    void UseShaderProgram(std::string shaderName);
    void DrawMesh(std::string meshName, std::string shaderName, bool instancing = false, uint32_t isntancingCount = 0);
    std::shared_ptr<Mesh> GetMesh(std::string meshName);

    std::shared_ptr<Shader> GetShaderProgram(std::string shaderName);
    unsigned int GetTextureID(std::string textureName);
    std::shared_ptr<Camera> GetCamera(std::string cameraName);

    // These are empty functions to be overloaded by the specific scene 
    virtual void SetupScene() {}
    virtual void RenderScene() {}

    std::unordered_map<std::string, std::shared_ptr<Camera>> Cameras;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> Meshes;
    std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
    std::unordered_map<std::string, unsigned int> Textures;

    std::unordered_map<std::string, std::vector<glm::vec3>> MeshParameters;
};

class SceneManager {
public:
    SceneManager() {}

    void RegisterScene(std::string sceneName, std::shared_ptr<Scene> scene)
    {
        Scenes[sceneName] = scene;
    }

    std::unordered_map<std::string, std::shared_ptr<Scene>> Scenes;
};

   