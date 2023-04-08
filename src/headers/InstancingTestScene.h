#pragma once

#include "SceneManager.h"

class InstancingTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string instancingShaderProgramName;
	std::string skyboxShaderProgramName;

	std::string containerDiffuseMap;
	std::string skyboxTexture;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};