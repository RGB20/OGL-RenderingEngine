#pragma once

#include "SceneManager.h"

class LightingTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string objectShaderProgramName;
	std::string stensilShaderProgramName;
	std::string lightShaderProgramName;

	std::string containerDiffuseMap;
	std::string containerSpecularMap;
	std::string wallDiffuseMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};