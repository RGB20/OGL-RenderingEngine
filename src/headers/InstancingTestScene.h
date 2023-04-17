#pragma once

#include "SceneManager.h"

class InstancingTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string instancingShaderProgramName;
	std::string objectShaderProgramName;
	std::string skyboxShaderProgramName;

	std::string skyboxTextureName;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	uint32_t asteroidCount;
};