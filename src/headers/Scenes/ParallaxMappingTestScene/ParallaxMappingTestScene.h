#pragma once

#include "headers/SceneManager.h"

class ParallaxMappingTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string ParallaxMappingShaderProgramName;

	std::string brickWallDiffuseMap;
	std::string brickWallNormalMap;
	std::string brickWallDisplacementMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;

	//test
	std::uint16_t time;
};