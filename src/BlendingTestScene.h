#pragma once

#include "SceneManager.h"

class BlendingTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string blendingShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};