#pragma once

#include "headers/SceneManager.h"

class NormalMappingTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string normalMappingShaderProgramName;
	std::string lightShaderProgramName;

	std::string brickWallDiffuseMap;
	std::string normalMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
};