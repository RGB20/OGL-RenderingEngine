#pragma once

#include "SceneManager.h"

class StensilTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

    std::string objectAndStensilShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};