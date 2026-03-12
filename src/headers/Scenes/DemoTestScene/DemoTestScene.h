#pragma once

#include "headers/SceneManager.h"

struct VoronoiCell
{
	int xboundmin;
	int xboundmax;
	int yboundmin;
	int yboundmax;

	std::vector<glm::vec2> samples;
};

class DemoTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);
	void DeltaTime(float deltaTime);
	void GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float heightScale, float lacunarity, float persistance, int octaves);
	void GenerateVoroniMap(std::shared_ptr<std::vector<float>> voronoiMap, size_t mapWidth, size_t mapHeight);
	void MergeHeightMaps(std::shared_ptr<std::vector<float>> perlinFBMNoise, std::shared_ptr<std::vector<float>> voronoiNoise, size_t mapWidth, size_t mapHeight);
	void HydrolicErosion();
	uint32_t erosionSimIterations;

	float accTime;
	float angleAroundCenter;
	size_t terrainMeshWidth;
	size_t terrainMeshHeight;
	std::shared_ptr<std::vector<float>> generatedHeightMap;
	std::shared_ptr<std::vector<float>> generatedVoronoiMap;

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