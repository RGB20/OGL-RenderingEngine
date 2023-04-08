#pragma once

#include "SceneManager.h"

class DepthTestingScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string depthTestShaderProgramName;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};