#pragma once

#include "headers/SceneManager.h"

class SkyboxTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string skyboxShaderProgramName;
	std::string blendingShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;
	std::string skyboxTexture;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};