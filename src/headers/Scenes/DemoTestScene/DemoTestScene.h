#pragma once

#include "headers/SceneManager.h"

struct Droplet {
	float x, y;
	float dirX = 0, dirY = 0;
	float speed = 1.0f;
	float water = 1.0f;
	float sediment = 0.0f;
};

struct TerrainChunk {
	std::string meshName;
	glm::vec3 minBounds;
	glm::vec3 maxBounds;
};

class DemoTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);
	void DeltaTime(float deltaTime);
	void GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float lacunarity, float persistence, int octaves);
	void GenerateVoroniMap(std::shared_ptr<std::vector<float>> voronoiMap, size_t mapWidth, size_t mapHeight);
	void MergeHeightMaps(std::shared_ptr<std::vector<float>> perlinFBMNoise, std::shared_ptr<std::vector<float>> voronoiNoise, size_t mapWidth, size_t mapHeight);
	void HydrolicErosion();
	void GenerateNormals();
	void BuildTerrainChunks();
	uint32_t erosionSimIterations;

	float accTime;
	float timeOfDay01;
	size_t terrainMeshWidth;
	size_t terrainMeshHeight;
	size_t heightMapWidth;
	size_t heightMapHeight;
	size_t terrainChunkSize;
	float heightScale;
	float waterLevel;
	float terrainMinHeight;
	float terrainMaxHeight;
	std::shared_ptr<std::vector<float>> generatedHeightMap;
	std::shared_ptr<std::vector<float>> generatedVoronoiMap;
	std::shared_ptr<std::vector<glm::vec4>> calculatedNormalMap;

	std::string skyboxShaderProgramName;
	std::string blendingShaderProgramName;
	std::string tessShaderProgramName;
	std::string tessNormalVisualizationShaderProgramName;
	std::string normalMapGenerationCS;
	std::string waterShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;
	std::string skyboxTexture;
	std::string heightMap;
	std::string customHeightMap;
	std::string customHeightBufferMap;
	std::string customNormalMap;
	std::string normalMap;


	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	std::shared_ptr<PatchInfo> patchInfo;
	std::vector<TerrainChunk> terrainChunks;
};
