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

enum class TerrainBrushType
{
	SmoothHill,
	SmoothPit,
	Flatten,
	Noise,
	RidgedMountain,
	Volcano,
	Canyon,
	Plateau,
	Mesa,
	Count
};

enum class BrushFalloffType
{
	Linear,
	Smooth,
	Sharp,
	Count
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

	void OnLeftMouseClick() { leftMouseHeld = true; }
	void OnLeftMouseRelease() { leftMouseHeld = false;}
	void ModifyHeightmap();
	void UpdateGPUHightmap();
	void UpdateNormals();
	void UpdateDirtyChunkBounds();

	float EvaluateBrushShape(glm::vec2 localPx, float distancePx, float radiusPx);
	float FractalNoise(float x, float y, int octaves, float frequency, uint32_t seed);
	float RidgeNoise(float x, float y, uint32_t seed);

	bool RayPlaneXZ(const glm::vec3& rayOrigin,const glm::vec3& rayDir,float planeY,glm::vec3& hit);
	bool SampleTerrainHeight(glm::vec2 worldXZ, float& outHeight);
	bool RayMarchTerrain(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float maxDistance, float stepSize, glm::vec3& hit);
	float BrushFalloff(float distance, float radius, float softness);
	void ApplyHeightBrush(glm::vec2 worldXZ, float radiusWorld, float strength);

	void DemoKeyPressed(uint16_t keyCode); 

	int minX = 0;
	int minY = 0;
	int maxX = 0;
	int maxY = 0;

	uint32_t erosionSimIterations;

	TerrainBrushType activeBrushType = TerrainBrushType::SmoothHill;
	BrushFalloffType activeFalloffType = BrushFalloffType::Smooth;

	bool dayNightCycle = true;
	float deltaTime;
	float accTime;
	float timeOfDay01;
	size_t terrainMeshWidth;
	size_t terrainMeshHeight;
	size_t heightMapWidth;
	size_t heightMapHeight;
	size_t terrainChunkSize;
	float terrainScale;
	float heightScale;
	float waterLevel;
	float terrainMinHeight;
	float terrainMaxHeight;
	bool heightMapDirty;
	bool leftMouseHeld = false;
	float brushRadius;
	bool brushHighlightActive;
	glm::vec2 brushCenterXZ;
	std::shared_ptr<std::vector<float>> generatedHeightMap;
	std::shared_ptr<std::vector<float>> generatedVoronoiMap;
	std::shared_ptr<std::vector<glm::vec4>> calculatedNormalMap;

	unsigned int depthMapFBO;
	unsigned int depthMap;
	const unsigned int SHADOW_WIDTH = SCR_WIDTH, SHADOW_HEIGHT = SCR_HEIGHT;

	std::string skyboxShaderProgramName;
	std::string blendingShaderProgramName;
	std::string tessShaderProgramName;
	std::string tessNormalVisualizationShaderProgramName;
	std::string normalMapGenerationCS;
	std::string updateNormalMapCS;
	std::string waterShaderProgramName;
	std::string terrainDepthShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;
	std::string skyboxTexture;
	std::string heightMap;
	std::string customHeightMap;
	std::string customNormalMap;
	std::string normalMap;


	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	std::shared_ptr<PatchInfo> patchInfo;
	std::vector<TerrainChunk> terrainChunks;
};
