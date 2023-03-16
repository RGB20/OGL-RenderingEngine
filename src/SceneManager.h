#pragma once
#include "Utilities.h"
#include "Camera.h"
#include "Model.h"
#include <unordered_map>

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

class Scene {
public:
    Scene() {}

    void AddShader(std::string shaderName, std::string vertexShaderPath, std::string fragmentShaderPath);
    void AddModel(std::string modelName, std::string modePath);
    void AddPresetModels(std::string modelName, DEFAULT_MODELS modelType);
    void AddCamera(std::string cameraName, std::shared_ptr<Camera> camera);
    void LoadTexture(std::string textureName, std::string textureFileName, std::string textureDir);
    void AddMeshParameter(std::string parameterName, std::vector<glm::vec3> parameterArray);

    void UseShaderProgram(std::string shaderName);
    void DrawModel(std::string modelName, std::string shaderName);

    std::shared_ptr<Shader> GetShaderProgram(std::string shaderName);
    unsigned int GetTextureID(std::string textureName);
    std::shared_ptr<Camera> GetCamera(std::string cameraName);

    // These are empty functions to be overloaded by the specific scene 
    virtual void SetupScene() {}
    virtual void RenderScene() {}

    std::unordered_map<std::string, std::shared_ptr<Camera>> Cameras;
    std::unordered_map<std::string, std::shared_ptr<Model>> Models;
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

   