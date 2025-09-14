#pragma once

#include "headers/SceneManager.h"

class ShadowMappingTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string shadowMappingShaderProgramName;
	std::string depthMappingShaderProgramName;
	std::string lightShaderProgramName; 

	std::string containerDiffuseMap;
	std::string containerSpecularMap;
	std::string wallDiffuseMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	unsigned int depthMapFBO;
	unsigned int depthMap;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	//test
	std::uint16_t time;
};