#pragma once
#include "Utilities.h"
#include "Camera.h"
#include "Model.h"
#include <unordered_map>

class Scene {
public:
    Scene() {}

    void AddShader(std::string shaderName, std::string vertexShaderPath, std::string fragmentShaderPath);
    void AddModel(std::string modelName, std::string modePath);
    void AddPresetModels(std::string modelName, DEFAULT_MODELS modelType);
    void AddCamera(std::string cameraName, std::shared_ptr<Camera> camera);
    void LoadTexture(std::string textureName, std::string textureFileName, std::string textureDir);

    void UseShaderProgram(std::string shaderName);
    void DrawModel(std::string modelName, std::string shaderName);

    std::shared_ptr<Shader> GetShaderProgram(std::string shaderName);
    unsigned int GetTextureID(std::string textureName);

    std::unordered_map<std::string, std::shared_ptr<Camera>> Cameras;
    std::unordered_map<std::string, std::shared_ptr<Model>> Models;
    std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
    std::unordered_map<std::string, unsigned int> Textures;
};


   