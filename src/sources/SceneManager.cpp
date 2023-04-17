#include "headers/SceneManager.h"

void Scene::AddShader(std::string shaderName, std::unordered_map<SHADER_TYPES, std::string> shaders)
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(shaderName, shaders);
	this->Shaders[shaderName] = shader;
}

void Scene::AddMesh(std::string meshName, std::string meshPath)
{
	Meshes[meshName] = std::make_shared<Mesh>(meshName, meshPath);
}

void Scene::AddPresetMesh(std::string meshName, DEFAULT_MESHES meshType)
{
	Meshes[meshName] = std::make_shared<Mesh>(meshType, meshName);
}

void Scene::DrawMesh(std::string meshName, std::string shaderName, bool instancing, uint32_t instancingCount)
{
	Meshes[meshName]->Draw(Shaders[shaderName], instancing, instancingCount);
}

std::shared_ptr<Mesh> Scene::GetMesh(std::string meshName)
{
	return Meshes[meshName];
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