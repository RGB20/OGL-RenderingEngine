#pragma once

#include "headers/SceneManager.h"

class HDRMappingTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string HDRMappingShaderProgramName;

	std::string WallDiffuseMap;
	std::string WallNormalMap;
	std::string WallDisplacementMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;

	//test
	std::uint16_t time;
};