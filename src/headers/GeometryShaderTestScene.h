#pragma once

#include "SceneManager.h"

class GeometryShaderTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string vertexNormalsShaderProgramName;
	std::string objectShaderProgramName;
	std::string skyboxShaderProgramName;

	std::string containerDiffuseMap;
	std::string skyboxTexture;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};