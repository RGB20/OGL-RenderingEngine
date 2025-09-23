#pragma once

#include "headers/SceneManager.h"

class ShadowMappingMegaScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string shadowMappingShaderProgramName;
	std::string depthMappingShaderProgramName;

	std::string containerDiffuseMap;
	std::string containerSpecularMap;
	std::string planeTexture;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	unsigned int depthMapFBO;
	unsigned int depthMap;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	// Demo
	bool dropTerrain = false;
	bool dropBoxes = false;
	bool addSkyBox = false;

	void DemoKeyPressed(uint16_t keyCode);

	//test
	float time;
};