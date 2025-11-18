#pragma once

#include "headers/SceneManager.h"

class DemoTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);
	void DeltaTime(float deltaTime);
	void GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float heightScale, float lacunarity, float persistance, int octaves);

	float accTime;
	float angleAroundCenter;

	std::string skyboxShaderProgramName;
	std::string blendingShaderProgramName;
	std::string tessShaderProgramName;
	std::string tessNormalVisualizationShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;
	std::string skyboxTexture;
	std::string heightMap;
	std::string customHeightMap;
	std::string normalMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	std::shared_ptr<PatchInfo> patchInfo;
};