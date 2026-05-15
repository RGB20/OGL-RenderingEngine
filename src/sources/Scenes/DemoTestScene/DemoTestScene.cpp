#include "headers/Scenes/DemoTestScene/DemoTestScene.h"
#include <algorithm>
#include <map>
#include "headers/Mesh.h"
#include <cmath>
#include <random>
#include <chrono>
#include <memory>
#include <headers/PerlinNoise.h>
#include "headers/PerlinNoiseLib.hpp"

namespace {
    struct FrustumPlane {
        glm::vec3 normal;
        float distance;
    };

    glm::vec4 GetMatrixRow(const glm::mat4& matrix, int row)
    {
        return glm::vec4(matrix[0][row], matrix[1][row], matrix[2][row], matrix[3][row]);
    }

    FrustumPlane NormalizePlane(const glm::vec4& plane)
    {
        float length = glm::length(glm::vec3(plane));
        return { glm::vec3(plane) / length, plane.w / length };
    }

    std::vector<FrustumPlane> ExtractFrustumPlanes(const glm::mat4& clipMatrix)
    {
        glm::vec4 row0 = GetMatrixRow(clipMatrix, 0);
        glm::vec4 row1 = GetMatrixRow(clipMatrix, 1);
        glm::vec4 row2 = GetMatrixRow(clipMatrix, 2);
        glm::vec4 row3 = GetMatrixRow(clipMatrix, 3);

        return {
            NormalizePlane(row3 + row0),
            NormalizePlane(row3 - row0),
            NormalizePlane(row3 + row1),
            NormalizePlane(row3 - row1),
            NormalizePlane(row3 + row2),
            NormalizePlane(row3 - row2)
        };
    }

    bool IsAABBInFrustum(const glm::vec3& minBounds, const glm::vec3& maxBounds, const std::vector<FrustumPlane>& frustumPlanes)
    {
        for (const FrustumPlane& plane : frustumPlanes)
        {
            glm::vec3 positiveVertex = minBounds;
            if (plane.normal.x >= 0.0f) positiveVertex.x = maxBounds.x;
            if (plane.normal.y >= 0.0f) positiveVertex.y = maxBounds.y;
            if (plane.normal.z >= 0.0f) positiveVertex.z = maxBounds.z;

            if (glm::dot(plane.normal, positiveVertex) + plane.distance < 0.0f)
            {
                return false;
            }
        }

        return true;
    }

    float DistanceToChunkSquared(const TerrainChunk& chunk, const glm::vec3& position)
    {
        glm::vec3 center = (chunk.minBounds + chunk.maxBounds) * 0.5f;
        glm::vec3 offset = center - position;
        return glm::dot(offset, offset);
    }

    float SmoothStepCPU(float edge0, float edge1, float value)
    {
        float t = glm::clamp((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
}

void DemoTestScene::SetupScene()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 100.0f, 0.0f)));


    // Object shader program and other handlers
    blendingShaderProgramName = "blendingShaderProgram";
    std::unordered_map<SHADER_TYPES, std::string> blendingShaders;

    blendingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    blendingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(blendingShaderProgramName, blendingShaders);

    skyboxShaderProgramName = "skyboxShaderProgram";
    std::unordered_map<SHADER_TYPES, std::string> skyboxShaders;

    skyboxShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\skyboxVertexShader.vs";
    skyboxShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\skyboxFragmentShader.fs";
    AddShader(skyboxShaderProgramName, skyboxShaders);

    tessShaderProgramName = "tessShaderProgram";
    std::unordered_map<SHADER_TYPES, std::string> tessShaders;

    tessShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\tessVertexShader.vs";
    tessShaders[SHADER_TYPES::TESS_CONTROL_SHADER] = GetCurrentDir() + "\\shaders\\tessControlShader.tcs";
    tessShaders[SHADER_TYPES::TESS_EVAL_SHADER] = GetCurrentDir() + "\\shaders\\tessEvalShader.tes";
    //tessShaders[SHADER_TYPES::GEOMETRY_SHADER] = GetCurrentDir() + "\\shaders\\tessGeometryShader.gs";
    tessShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\tessFragmentShader.fs";
    AddShader(tessShaderProgramName, tessShaders);

    waterShaderProgramName = "waterShaderProgram";
    std::unordered_map<SHADER_TYPES, std::string> waterShaders;

    waterShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\waterVertexShader.vs";
    waterShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\waterFragmentShader.fs";
    AddShader(waterShaderProgramName, waterShaders);

    normalMapGenerationCS = "GenerateNormalsCS";
    std::unordered_map<SHADER_TYPES, std::string> csShaders;

    csShaders[SHADER_TYPES::COMPUT_SHADER] = GetCurrentDir() + "\\Shaders\\ComputeShader.comp";
    AddShader(normalMapGenerationCS, csShaders);

    //tessNormalVisualizationShaderProgramName = "tessShaderProgram_NormalVisualization";
    //std::unordered_map<SHADER_TYPES, std::string> tessNormalVisualizationShaders;

    //tessNormalVisualizationShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\tessVertexShader.vs";
    //tessNormalVisualizationShaders[SHADER_TYPES::TESS_CONTROL_SHADER] = GetCurrentDir() + "\\shaders\\tessControlShader.tcs";
    //tessNormalVisualizationShaders[SHADER_TYPES::TESS_EVAL_SHADER] = GetCurrentDir() + "\\shaders\\normalVisualizationTessEvalShader.tes";
    //tessNormalVisualizationShaders[SHADER_TYPES::GEOMETRY_SHADER] = GetCurrentDir() + "\\shaders\\normalVisualizationTessGeometryShader.gs";
    //tessNormalVisualizationShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\normalVisualizationTessFragmentShader.fs";
    //AddShader(tessNormalVisualizationShaderProgramName, tessNormalVisualizationShaders);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    wallDiffuseMap = "wallDiffuseMap";
    grassTexture = "grassTexture";
    transparentWindowTexture = "transparentWindowTexture";
    skyboxTexture = "skyboxTexture";
    //heightMap = "heightMap";
    customHeightMap = "customHeightMap";
    customHeightBufferMap = "customHeightBufferMap";
    normalMap = "normalMap";

    bool HDR = true;
    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory, HDR);
    LoadTexture(wallDiffuseMap, "wall.jpg", textureDirectory, HDR);
    LoadTexture(grassTexture, "grass.png", textureDirectory, HDR);
    LoadTexture(transparentWindowTexture, "blending_transparent_window.png", textureDirectory, HDR);
    //LoadTexture(heightMap, "BOTW_HeightMap.png", textureDirectory, HDR);
    //LoadTexture(normalMap, "BOTW_NormalMap.png", textureDirectory, false);

    // Generated unpreturbed height map
    terrainMeshWidth = 1600;
    terrainMeshHeight = 1600;// 4096;
    terrainChunkSize = 64;
    heightScale = 125.0f;
    waterLevel = 16.0f;
    terrainMinHeight = 0.0f;
    terrainMaxHeight = heightScale;
    float lacunarify = 2.0;
    float persistance = 0.5;
    int octaves = 1;
    generatedHeightMap = std::make_shared<std::vector<float>>(terrainMeshWidth * terrainMeshHeight, 0);
    GenerateTerrainHeightMap(generatedHeightMap, terrainMeshWidth, terrainMeshHeight, lacunarify, persistance, octaves);

    // Generate Voronoi map
    generatedVoronoiMap = std::make_shared<std::vector<float>>(terrainMeshWidth * terrainMeshHeight, 0);
    GenerateVoroniMap(generatedVoronoiMap, terrainMeshWidth, terrainMeshHeight);

    MergeHeightMaps(generatedHeightMap, generatedVoronoiMap, terrainMeshWidth, terrainMeshHeight);

    int nComponents = 1;
    HDR = false;
    LoadTextureRaw(customHeightMap, generatedHeightMap->data(), terrainMeshWidth, terrainMeshHeight, nComponents, HDR);
    
    //Loading a pre-calculated height map
    //std::string textureDirectory_custom = GetCurrentDir();
    //LoadTexture(customHeightMap, "f16o4_51684521.bmp", textureDirectory_custom, HDR);
    //LoadTexture(customHeightMap, "BOTW_HeightMap.png", textureDirectory, HDR);

    //std::string skyboxtextureDirectory = GetCurrentDir() + "\\textures\\skyboxTextures\\OceanAndSky\\";

    //std::vector<std::string> cubemapFaces;
    //cubemapFaces.push_back("right.jpg");
    //cubemapFaces.push_back("left.jpg");
    //cubemapFaces.push_back("top.jpg");
    //cubemapFaces.push_back("bottom.jpg");
    //cubemapFaces.push_back("front.jpg");
    //cubemapFaces.push_back("back.jpg");
    //LoadCubeMapTexture("skyboxCubeMap", cubemapFaces, skyboxtextureDirectory);

    GetShaderProgram(blendingShaderProgramName)->setInt("material.diffuse", 0);
    GetShaderProgram(skyboxShaderProgramName)->setInt(skyboxTexture, 0);
    GetShaderProgram(tessShaderProgramName)->setInt(customHeightMap, 0);
    //GetShaderProgram(tessShaderProgramName)->setInt("normalMap", 1);

    //UseShaderProgram(tessNormalVisualizationShaderProgramName);
    //GetShaderProgram(tessNormalVisualizationShaderProgramName)->setInt("heightMap", 0);
    //GetShaderProgram(tessNormalVisualizationShaderProgramName)->setInt("normalMap", 1);

    // Perform Erosion
    erosionSimIterations = 1;
    HydrolicErosion();

    auto heightRange = std::minmax_element(generatedHeightMap->begin(), generatedHeightMap->end());
    terrainMinHeight = *heightRange.first;
    terrainMaxHeight = *heightRange.second;

    // Load height map for the compute shader
    LoadBuffer(customHeightBufferMap, generatedHeightMap, (terrainMeshWidth * terrainMeshHeight) * sizeof(float));
    // Load the empty normal map
    calculatedNormalMap = std::make_shared<std::vector<glm::vec4>>(terrainMeshWidth * terrainMeshHeight, glm::vec4(0));
    customNormalMap = "Generated Normal Map";
    LoadBuffer(customNormalMap, calculatedNormalMap, (terrainMeshWidth * terrainMeshHeight) * sizeof(glm::vec4));

    // Generate the normal map using a compute shader
    GenerateNormals();


    // Add/Load Models
    //AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);
    AddPresetMesh("skyDome", DEFAULT_MESHES::HEMISPHERE);

    patchInfo = std::make_shared<PatchInfo>();
    patchInfo->resX = static_cast<unsigned int>(terrainChunkSize + 1);
    patchInfo->resY = static_cast<unsigned int>(terrainChunkSize + 1);
    patchInfo->patchPrimType = PATCH_PRIM_TYPE::TRI_MESH;
    BuildTerrainChunks();

    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));

    std::vector<glm::vec3> customPlaneMeshPosition;
    customPlaneMeshPosition.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

    std::vector<glm::vec3> windowPanelPositions;
    windowPanelPositions.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    windowPanelPositions.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    windowPanelPositions.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    windowPanelPositions.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    windowPanelPositions.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    std::vector<glm::vec3> skyDomePosition;
    skyDomePosition.push_back(glm::vec3(0.0f)); // Starting the sky dome at (0,0,0) we will move it woth the camera position

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["cubePositions"] = cubePositions;
    sceneAttributes["windowPanelPositions"] = windowPanelPositions;
    sceneAttributes["customPlaneMeshPosition"] = customPlaneMeshPosition;
    sceneAttributes["skyDomePosition"] = skyDomePosition;

    accTime = 0;
}

void DemoTestScene::MergeHeightMaps(std::shared_ptr<std::vector<float>> perlinFBMNoise, std::shared_ptr<std::vector<float>> voronoiNoise, size_t mapWidth, size_t mapHeight)
{
#ifdef _DEBUG
    Image image{ mapWidth, mapHeight };
#endif
    float mountainStart = 0.56f;
    float mountainFull = 0.82f;
    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            int idx = x + y * mapWidth;
            float meadowHeight = (*perlinFBMNoise)[idx];
            float mountainMask = SmoothStepCPU(mountainStart, mountainFull, meadowHeight);
            float ridgeNoise = (*voronoiNoise)[idx];
            float ridgeHeight = glm::pow(glm::clamp(ridgeNoise, 0.0f, 1.0f), 1.65f);
            float mergedNoiseHeight = glm::mix(meadowHeight, glm::max(meadowHeight, ridgeHeight), mountainMask * 0.45f);

            // Smoothly step the height down to 0 starting 10 units from the edges
            float distToEdgeX = std::min(static_cast<float>(x), static_cast<float>(mapWidth - 1 - x));
            float distToEdgeY = std::min(static_cast<float>(y), static_cast<float>(mapHeight - 1 - y));
            float distToEdge = std::min(distToEdgeX, distToEdgeY);
            float edgeMask = SmoothStepCPU(0.0f, 10.0f, distToEdge);
            mergedNoiseHeight *= edgeMask;

#ifdef _DEBUG
            const RGB color(mergedNoiseHeight);
            image.set(x, y, color);
#endif
            (*perlinFBMNoise)[idx] = mergedNoiseHeight * heightScale;
        }
    }

#ifdef _DEBUG
    std::stringstream ss;
    ss << "Perlin_Voronoi_Merged" << ".bmp";

    if (image.saveBMP(ss.str()))
    {
        std::cout << "...saved \"" << ss.str() << "\"\n";
    }
    else
    {
        std::cout << "...failed\n";
    }
#endif
}

float closestSeedIndex(int px, int py, int seedDimensionXY, const std::vector<glm::vec2>& seeds, float mapWidth, float mapHeight) {
    int bestIndex = 0;
    float bestDist = FLT_MAX;
    float secondBestDist = FLT_MAX;
    int quadrentSizeX = mapWidth / seedDimensionXY;
    int quadrentSizeY = mapHeight / seedDimensionXY;
    float maxPossibleDistance = std::sqrtf(mapWidth * mapWidth + mapHeight * mapHeight);

    for (int i = 0; i < seedDimensionXY; i++) {
        float dx = px - seeds[i].x;
        float dy = py - seeds[i].y;
        float dist = std::sqrtf(dx * dx + dy * dy);

        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }

    bestDist = bestDist / maxPossibleDistance;
    // Rasing the distance to a power to accentuate the edges and darken the points closer to the seeds
    bestDist = glm::clamp(float(glm::pow(bestDist, 0.4)), 0.0f, 1.0f);

    return bestDist;
}

float randomInRange(float minVal, float maxVal) {
    float t = rand() / float(RAND_MAX);   // t in [0,1]
    return minVal + t * (maxVal - minVal);
}

void DemoTestScene::GenerateVoroniMap(std::shared_ptr<std::vector<float>> voronoiMap, size_t mapWidth, size_t mapHeight)
{
#ifdef _DEBUG
    Image image{ mapWidth, mapHeight };
#endif
    
    std::uint32_t seed = 231842352;

    const siv::PerlinNoise perlin{ seed };

    // Generating NxN number of seeds for Voronoi noise
    int seedDimensionXY = 16;

    //std::vector<std::vector<glm::vec2>> seeds;
    std::vector<glm::vec2> seeds;
    float jitter = 0.5;
    for (int x = 0; x < seedDimensionXY; x++)
    {
        float samplePointx = randomInRange(0.0f, mapWidth);// x + jitter * std::abs(randomInRange(0.0f, 1.0f) - randomInRange(0.0f, 1.0f));
        float samplePointy = randomInRange(0.0f, mapHeight);// y + jitter * std::abs(randomInRange(0.0f, 1.0f) - randomInRange(0.0f, 1.0f));
        seeds.push_back(glm::vec2(samplePointx, samplePointy));
    }

    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            int idx = x + y * mapWidth;
            float minDistance = closestSeedIndex(x, y, seedDimensionXY, seeds, mapWidth, mapHeight);
            (*voronoiMap)[idx] = 1.0f - minDistance;
#ifdef _DEBUG
            const RGB color(minDistance);
            image.set(x, y, color);
#endif
        }
    }

#ifdef _DEBUG
    std::stringstream ss;
    ss << "Voronoimap_" << ".bmp";

    if (image.saveBMP(ss.str()))
    {
        std::cout << "...saved \"" << ss.str() << "\"\n";
    }
    else
    {
        std::cout << "...failed\n";
    }
#endif
}

void DemoTestScene::GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float lacunarity, float persistance, int octaves)
{

#ifdef _DEBUG
    Image image{ mapWidth, mapWidth };
#endif
    std::uint32_t seed = 231842352;

    const siv::PerlinNoise perlin{ seed };
    const double meadowFx = 2.6 / mapWidth;
    const double meadowFy = 2.6 / mapHeight;
    const double hillFx = 7.5 / mapWidth;
    const double hillFy = 7.5 / mapHeight;
    const double ridgeFx = 18.0 / mapWidth;
    const double ridgeFy = 18.0 / mapHeight;
    const double mountainFx = 3.8 / mapWidth;
    const double mountainFy = 3.8 / mapHeight;

    // Parameters for stepped mountains
    const int numSteps = 18; // Number of height steps (increase for more steps, smoother terrain)
    const float stepSmoothing = 0.5f; // Smoothing between steps (0 = sharp, 1 = very smooth)

    for (std::int32_t x = 0; x < mapWidth; ++x)
    {
        for (std::int32_t y = 0;y < mapHeight; ++y)
        {
            float meadow = static_cast<float>(perlin.octave2D_01(x * meadowFx, y * meadowFy, 4));
            float hills = static_cast<float>(perlin.octave2D_01(x * hillFx + 31.7, y * hillFy - 18.4, 5));
            float ridges = static_cast<float>(perlin.octave2D_01(x * ridgeFx - 73.1, y * ridgeFy + 9.2, 4));
            float mountainRegion = static_cast<float>(perlin.octave2D_01(x * mountainFx + 101.0, y * mountainFy - 47.0, 3));

            float meadowFloor = glm::pow(glm::clamp(meadow, 0.0f, 1.0f), 2.45f) * 0.34f;
            float rollingMeadow = meadowFloor + (hills - 0.5f) * 0.08f;

            float gradualMountain = SmoothStepCPU(0.48f, 0.88f, mountainRegion);
            float sharpMountain = SmoothStepCPU(0.68f, 0.79f, mountainRegion);
            float mountainMask = glm::mix(gradualMountain, sharpMountain, SmoothStepCPU(0.52f, 0.78f, ridges));
            
            float mountainHeight = 0.42f + glm::pow(glm::clamp(ridges, 0.0f, 1.0f), 1.45f) * 0.58f;

            float noiseValue = glm::mix(rollingMeadow, mountainHeight, mountainMask);
            noiseValue = glm::clamp(noiseValue, 0.0f, 1.0f);
            
            // Create stepped appearance with smooth transitions to avoid edge peaks
            float stepValue = noiseValue * numSteps;
            float stepIndex = glm::floor(stepValue);
            float stepFrac = stepValue - stepIndex;
            
            // Use smoothstep with flattened middle to create plateaus, then blend back
            float smoothedFrac = SmoothStepCPU(0.0f, 1.0f, stepFrac);
            // Further flatten by keeping most values near step boundaries
            smoothedFrac = glm::mix(stepFrac, smoothedFrac, stepSmoothing);
            
            noiseValue = glm::clamp((stepIndex + smoothedFrac) / numSteps, 0.0f, 1.0f);
            
#ifdef _DEBUG
            const RGB color(noiseValue);
            image.set(x, y, color);
#endif
            (*heightMap)[x + y * mapWidth] = noiseValue;
        }
    }

#ifdef _DEBUG
    std::stringstream ss;
    ss << "Heightmap_SteppedMountains_" << seed << ".bmp";

    if (image.saveBMP(ss.str()))
    {
        std::cout << "...saved \"" << ss.str() << "\"\n";
    }
    else
    {
        std::cout << "...failed\n";
    }
#endif
}

void DemoTestScene::DeltaTime(float deltaTime)
{
    accTime += deltaTime;
    float dayDurationSeconds = 120.0f; // Full day cycle every 2 minutes
    timeOfDay01 = std::fmod(accTime, dayDurationSeconds) / dayDurationSeconds;
}

void DemoTestScene::HydrolicErosion()
{
    if (erosionSimIterations > 0) {
        erosionSimIterations -= 1;

        int maxErosionSteps = 1200;
        int numberOfDroplets = 1000000;

        // Tuning parameters for a more subtle and smoother effect
        float inertia = 0.4f;         // Increased inertia helps droplets "glide" over and distribute sediment over longer distances
        float erodeSpeed = 0.01f;     // Further reduced to keep terrain features sharp but smoothed
        float depositSpeed = 0.01f;   // Lowered to prevent lumpy sediment piles
        float evaporateSpeed = 0.02f; // Droplets dry up faster, which helps smear sediment more thinly
        float gravity = 4.0f;

        for (int dropletNumber = 0; dropletNumber < numberOfDroplets; dropletNumber++)
        {
            Droplet d;
            d.x = randomInRange(0.0f, (float)terrainMeshWidth - 1.0f);
            d.y = randomInRange(0.0f, (float)terrainMeshHeight - 1.0f);
            d.speed = 1.0f;
            d.water = 1.0f;
            d.sediment = 0.0f;
            d.dirX = 0;
            d.dirY = 0;

            for (int step = 0; step < maxErosionSteps; step++) {
                int ix = (int)d.x;
                int iy = (int)d.y;
                float u = d.x - ix;
                float v = d.y - iy;

                if (d.x < 1 || d.x > terrainMeshWidth - 2 || d.y < 1 || d.y > terrainMeshHeight - 2)
                    break;

                // Calculate height and gradient using bilinear interpolation for smoothness
                float h00 = (*generatedHeightMap)[iy * terrainMeshWidth + ix];
                float h10 = (*generatedHeightMap)[iy * terrainMeshWidth + (ix + 1)];
                float h01 = (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + ix];
                float h11 = (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + (ix + 1)];

                float gradX = (h10 - h00) * (1 - v) + (h11 - h01) * v;
                float gradY = (h01 - h00) * (1 - u) + (h11 - h10) * u;
                float oldH = h00 * (1 - u) * (1 - v) + h10 * u * (1 - v) + h01 * (1 - u) * v + h11 * u * v;

                // Update direction (inertia)
                d.dirX = d.dirX * inertia - gradX * (1.0f - inertia);
                d.dirY = d.dirY * inertia - gradY * (1.0f - inertia);

                // Normalize direction
                float len = std::sqrt(d.dirX * d.dirX + d.dirY * d.dirY);
                if (len != 0) {
                    d.dirX /= len;
                    d.dirY /= len;
                }

                // Move droplet
                d.x += d.dirX;
                d.y += d.dirY;

                if (d.x < 0 || d.x >= terrainMeshWidth - 1 || d.y < 0 || d.y >= terrainMeshHeight - 1)
                    break;

                // Get new height at new position using bilinear sampling
                int nix = (int)d.x;
                int niy = (int)d.y;
                float nu = d.x - nix;
                float nv = d.y - niy;
                float nh00 = (*generatedHeightMap)[niy * terrainMeshWidth + nix];
                float nh10 = (*generatedHeightMap)[niy * terrainMeshWidth + (nix + 1)];
                float nh01 = (*generatedHeightMap)[(niy + 1) * terrainMeshWidth + nix];
                float nh11 = (*generatedHeightMap)[(niy + 1) * terrainMeshWidth + (nix + 1)];
                float newH = nh00 * (1 - nu) * (1 - nv) + nh10 * nu * (1 - nv) + nh01 * (1 - nu) * nv + nh11 * nu * nv;

                float deltaH = newH - oldH;

                // Sediment capacity
                // Using slope magnitude (gradient length) provides a more stable capacity than local height difference
                float slope = std::sqrt(gradX * gradX + gradY * gradY);
                float capacity = std::max(slope * d.speed * d.water * 4.0f, 0.01f);

                // Calculate a multiplier to stop erosion at water level and fade it in
                float fadeMargin = 15.0f; // Height range above waterLevel where erosion fades in
                float erosionMultiplier = glm::clamp((oldH - waterLevel) / fadeMargin, 0.0f, 1.0f);

                if (deltaH > 0) {
                    // Droplet is moving uphill: use a much smaller factor (0.02f) to fill pits very subtly
                    float amountToDeposit = std::min(deltaH, d.sediment) * 0.02f;
                    amountToDeposit *= erosionMultiplier;

                    (*generatedHeightMap)[iy * terrainMeshWidth + ix] += amountToDeposit * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * terrainMeshWidth + (ix + 1)] += amountToDeposit * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + ix] += amountToDeposit * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + (ix + 1)] += amountToDeposit * u * v;
                    d.sediment -= amountToDeposit;
                }
                else if (d.sediment > capacity) {
                    // Oversaturated: standard deposition using the tuned depositSpeed
                    float amountToDeposit = (d.sediment - capacity) * depositSpeed;
                    amountToDeposit *= erosionMultiplier;

                    (*generatedHeightMap)[iy * terrainMeshWidth + ix] += amountToDeposit * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * terrainMeshWidth + (ix + 1)] += amountToDeposit * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + ix] += amountToDeposit * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + (ix + 1)] += amountToDeposit * u * v;
                    d.sediment -= amountToDeposit;
                }
                else {
                    // Erode - distribute across 4 neighbors at old position for smoothness
                    float amountToErode = std::min((capacity - d.sediment) * erodeSpeed, -deltaH);
                    amountToErode *= erosionMultiplier;

                    (*generatedHeightMap)[iy * terrainMeshWidth + ix] -= amountToErode * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * terrainMeshWidth + (ix + 1)] -= amountToErode * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + ix] -= amountToErode * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + (ix + 1)] -= amountToErode * u * v;
                    d.sediment += amountToErode;
                }

                // Update droplet
                d.speed = std::sqrt(std::max(0.0f, d.speed * d.speed + deltaH * gravity));
                d.water *= (1.0f - evaporateSpeed);

                if (d.water < 0.01f || erosionMultiplier < 0.01f)
                    break;
            }
        }

        glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, terrainMeshWidth, terrainMeshHeight, GL_RED, GL_FLOAT, generatedHeightMap->data());
        return;
    }
}

void DemoTestScene::GenerateNormals()
{
    UseShaderProgram(normalMapGenerationCS);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, GetBufferID(customHeightBufferMap)); // Binding 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetBufferID(customNormalMap)); // Binding 1

    // Match ComputeShader.comp's local_size_x.
    int gridDimX = static_cast<int>(((terrainMeshWidth * terrainMeshHeight) + 63) / 64);

    unsigned int computeProgram = GetShaderProgramID(normalMapGenerationCS);
    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimX"), static_cast<int>(terrainMeshWidth));
    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimY"), static_cast<int>(terrainMeshHeight));
    glUniform1f(glGetUniformLocation(computeProgram, "spacing"), 0.1f);

    glDispatchCompute(gridDimX, 1, 1);

    // ensure writes are visible to subsequent reads/copies
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

#ifdef _DEBUG
    // Read back the normal map and dump it into an image
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GetBufferID(customNormalMap));

    GLsizeiptr size = terrainMeshWidth * terrainMeshHeight * sizeof(glm::vec4);
    void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, GL_MAP_READ_BIT);

    if (ptr) {
        glm::vec4* normalsBuffer = reinterpret_cast<glm::vec4*>(ptr);

        Image image{ terrainMeshWidth, terrainMeshHeight};

        for (std::int32_t x = 0; x < terrainMeshWidth; ++x)
        {
            for (std::int32_t y = 0; y < terrainMeshHeight; ++y)
            {
                glm::vec4 temp = normalsBuffer[x + y * terrainMeshWidth];
                RGB normal((temp.r * 0.5f) + 0.5f, (temp.g * 0.5f) + 0.5f, (temp.b * 0.5f) + 0.5f);
                image.set(x, y, normal);
            }
        }

        std::stringstream ss;
        ss << "Generated_Normals.bmp";

        if (image.saveBMP(ss.str()))
        {
            std::cout << "...saved \"" << ss.str() << "\"\n";
        }
        else
        {
            std::cout << "...failed\n";
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
#endif
}

void DemoTestScene::BuildTerrainChunks()
{
    terrainChunks.clear();

    const float terrainOriginX = -static_cast<float>(terrainMeshWidth) * 0.5f;
    const float terrainOriginZ = -static_cast<float>(terrainMeshHeight) * 0.5f;
    const size_t maxX = terrainMeshWidth - 1;
    const size_t maxY = terrainMeshHeight - 1;
    const size_t chunkCountX = (maxX + terrainChunkSize - 1) / terrainChunkSize;
    const size_t chunkCountY = (maxY + terrainChunkSize - 1) / terrainChunkSize;

    for (size_t chunkY = 0; chunkY < chunkCountY; ++chunkY)
    {
        for (size_t chunkX = 0; chunkX < chunkCountX; ++chunkX)
        {
            size_t startX = chunkX * terrainChunkSize;
            size_t startY = chunkY * terrainChunkSize;
            size_t endX = std::min(startX + terrainChunkSize, maxX);
            size_t endY = std::min(startY + terrainChunkSize, maxY);
            size_t vertexCountX = endX - startX + 1;
            size_t vertexCountY = endY - startY + 1;

            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            std::vector<Texture> textures;
            vertices.reserve(vertexCountX * vertexCountY);
            indices.reserve((vertexCountX - 1) * (vertexCountY - 1) * 6);

            float chunkMinHeight = terrainMaxHeight;
            float chunkMaxHeight = terrainMinHeight;

            for (size_t localY = 0; localY < vertexCountY; ++localY)
            {
                for (size_t localX = 0; localX < vertexCountX; ++localX)
                {
                    size_t sampleX = startX + localX;
                    size_t sampleY = startY + localY;
                    float height = (*generatedHeightMap)[sampleX + sampleY * terrainMeshWidth];
                    chunkMinHeight = std::min(chunkMinHeight, height);
                    chunkMaxHeight = std::max(chunkMaxHeight, height);

                    Vertex vertex;
                    vertex.Position = glm::vec3(terrainOriginX + static_cast<float>(sampleX), 0.0f, terrainOriginZ + static_cast<float>(sampleY));
                    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    vertex.TexCoords = glm::vec2(
                        static_cast<float>(sampleX) / static_cast<float>(terrainMeshWidth - 1),
                        static_cast<float>(sampleY) / static_cast<float>(terrainMeshHeight - 1)
                    );
                    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                    vertex.BiTangent = glm::vec3(0.0f, 0.0f, 1.0f);

                    vertices.push_back(vertex);
                }
            }

            for (size_t localY = 0; localY < vertexCountY - 1; ++localY)
            {
                for (size_t localX = 0; localX < vertexCountX - 1; ++localX)
                {
                    unsigned int bottomLeft = static_cast<unsigned int>(localX + localY * vertexCountX);
                    unsigned int bottomRight = bottomLeft + 1;
                    unsigned int topLeft = static_cast<unsigned int>(localX + (localY + 1) * vertexCountX);
                    unsigned int topRight = topLeft + 1;

                    indices.push_back(bottomLeft);
                    indices.push_back(topLeft);
                    indices.push_back(bottomRight);

                    indices.push_back(bottomRight);
                    indices.push_back(topLeft);
                    indices.push_back(topRight);
                }
            }

            std::shared_ptr<Mesh> chunkMesh = std::make_shared<Mesh>();
            chunkMesh->SetupMesh(vertices, indices, textures);

            TerrainChunk chunk;
            chunk.meshName = "terrain_chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkY);
            chunk.minBounds = glm::vec3(terrainOriginX + static_cast<float>(startX), chunkMinHeight, terrainOriginZ + static_cast<float>(startY));
            chunk.maxBounds = glm::vec3(terrainOriginX + static_cast<float>(endX), chunkMaxHeight, terrainOriginZ + static_cast<float>(endY));

            AddCustomMesh(chunk.meshName, chunkMesh);
            terrainChunks.push_back(chunk);
        }
    }
}

void DemoTestScene::RenderScene(unsigned int deferredQuadFrameBuffer)
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredQuadFrameBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render Objects
    UseShaderProgram(blendingShaderProgramName);
    std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(blendingShaderProgramName);

    // VS stage Uniform inputs
    // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    // You can set the uniforms once or every frame
    // View
    objectShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);
    objectShaderProgram->setMat4("projection", projection);

    // FS stage Uniform inputs
    objectShaderProgram->setBool("texturing", true);

    // Planes VAO
    // Set the material diffuse and specular maps
    // texture1 - Material diffuse
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("wallDiffuseMap"));

    // --------------------------------------------------------------
    // Use TESS shader program to render the terrain
    UseShaderProgram(tessShaderProgramName);
    std::shared_ptr<Shader> tessShaderProgram = GetShaderProgram(tessShaderProgramName);
   
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("normalMap"));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetBufferID(customNormalMap));

    // Set view matrix
    glm::mat4 terrainView = GetCamera("MainCamera")->GetViewMatrix();
    tessShaderProgram->setMat4("view", terrainView);
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);
    tessShaderProgram->setMat4("projection", projection);

    // Custom plane mesh
    glDisable(GL_CULL_FACE);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["customPlaneMeshPosition"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    tessShaderProgram->setMat4("model", model);
    tessShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
    tessShaderProgram->setInt("terrainDimX", static_cast<int>(terrainMeshWidth));
    tessShaderProgram->setInt("terrainDimY", static_cast<int>(terrainMeshHeight));
    tessShaderProgram->setFloat("terrainMinHeight", terrainMinHeight);
    tessShaderProgram->setFloat("terrainMaxHeight", terrainMaxHeight);
    tessShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

    tessShaderProgram->setFloat("timeOfDay01", 1.0f);// timeOfDay01);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::vector<FrustumPlane> terrainFrustumPlanes = ExtractFrustumPlanes(projection * terrainView);
    std::vector<const TerrainChunk*> visibleChunks;
    visibleChunks.reserve(terrainChunks.size());

    for (const TerrainChunk& chunk : terrainChunks)
    {
        if (IsAABBInFrustum(chunk.minBounds, chunk.maxBounds, terrainFrustumPlanes))
        {
            DrawMesh(chunk.meshName, tessShaderProgramName, false, 0, patchInfo);
        }
    }
    
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Draw SKYBOX before the transparent meshes
    glCullFace(GL_FRONT); 
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    UseShaderProgram(skyboxShaderProgramName);
    std::shared_ptr<Shader> skyboxShaderProgram = GetShaderProgram(skyboxShaderProgramName);

    // Remove translation by taking the 3x3 rotation matrix and converting back to 4x4
    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(GetCamera("MainCamera")->GetViewMatrix()));
    skyboxShaderProgram->setMat4("viewNoTranslate", viewNoTrans); 

    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::scale(skyModel, glm::vec3(500.0f)); // Scale up so it surrounds the scene
    skyboxShaderProgram->setMat4("model", skyModel);

    // Use the same far plane as the terrain (10000.0) to ensure the skydome is not clipped
    projection = glm::perspective(glm::radians(ZOOM), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    skyboxShaderProgram->setMat4("projection", projection); 
    //skyboxShaderProgram->setVec3("cameraPos", GetCamera("MainCamera")->Position);
    
    float dayProgress = timeOfDay01 * 2.0f * 3.14159265f;
    float maxElevation = 3.14159265f * 0.25f;
    float elevation = std::sin(dayProgress) * maxElevation;
    float azimuth = dayProgress;
    glm::vec3 sunDirection = {cos(elevation) * cos(azimuth),sin(elevation),cos(elevation) * sin(azimuth)};
    skyboxShaderProgram->setVec3("sunDirection", sunDirection);

    // draw the skybox hemisphere
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, GetTextureID("skyboxCubeMap"));
    DrawMesh("skyDome", skyboxShaderProgramName);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glCullFace(GL_BACK);

    // Rendering the water
    UseShaderProgram(waterShaderProgramName);
    std::shared_ptr<Shader> waterShaderProgram = GetShaderProgram(waterShaderProgramName);
    waterShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);
    waterShaderProgram->setMat4("projection", projection);
    waterShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);
    waterShaderProgram->setFloat("time", accTime);
    waterShaderProgram->setInt("heightMap", 0);
    waterShaderProgram->setFloat("heightScale", heightScale);
    waterShaderProgram->setFloat("waterLevel", waterLevel);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));

    glm::mat4 waterModel = glm::mat4(1.0f);
    waterModel = glm::translate(waterModel, glm::vec3(0.0f, waterLevel, 0.0f));
    waterModel = glm::scale(waterModel, glm::vec3(static_cast<float>(terrainMeshWidth) * 0.5f, 1.0f, static_cast<float>(terrainMeshHeight) * 0.5f));
    waterShaderProgram->setMat4("model", waterModel);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    DrawMesh("plane", waterShaderProgramName);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}
