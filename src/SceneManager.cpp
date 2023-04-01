#include "SceneManager.h"

void Scene::AddShader(std::string shaderName, std::string vertexShaderPath, std::string fragmentShaderPath) 
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(shaderName, vertexShaderPath.c_str(), fragmentShaderPath.c_str());
	this->Shaders[shaderName] = shader;
}

void Scene::AddModel(std::string modelName, std::string modePath)
{
	std::shared_ptr<Model> model = std::make_shared<Model>(modelName, modePath);
	this->Models[modelName] = model;
}

void Scene::AddPresetModels(std::string modelName, DEFAULT_MODELS modelType) 
{
	std::shared_ptr<Model> model = std::make_shared<Model>(modelType, modelName);
	this->Models[modelName] = model;
}

void Scene::LoadTexture(std::string textureName, std::string textureFileName, std::string textureDir)
{
	unsigned int textureID = TextureFromFile(textureFileName.c_str(), textureDir.c_str());
	this->Textures[textureName] = textureID;
}

void Scene::LoadCubeMapTexture(std::string textureName, std::vector<std::string> faces, std::string textureDir)
{
	unsigned int textureID = LoadCubeMapFromFile(faces, textureDir);
	this->Textures[textureName] = textureID;
}

void Scene::AddCamera(std::string cameraName, std::shared_ptr<Camera> camera)
{
	Cameras[cameraName] = camera;
}

void Scene::DrawModel(std::string modelName, std::string shaderName)
{
	Models[modelName]->Draw(Shaders[shaderName]);
}

void Scene::UseShaderProgram(std::string shaderName)
{
	Shaders[shaderName]->use();
}

std::shared_ptr<Shader> Scene::GetShaderProgram(std::string shaderName)
{
	return Shaders[shaderName];
}

unsigned int Scene::GetTextureID(std::string textureName) 
{
	return Textures[textureName];
}

void Scene::AddMeshParameter(std::string parameterName, std::vector<glm::vec3> parameterArray)
{
	MeshParameters[parameterName] = parameterArray;
}

std::shared_ptr<Camera> Scene::GetCamera(std::string cameraName)
{
	return Cameras[cameraName];
}