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
#include <GLFW/glfw3.h>
#include <limits>

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
    std::unordered_map<SHADER_TYPES, std::string> GenerateNormalsShader;

    GenerateNormalsShader[SHADER_TYPES::COMPUT_SHADER] = GetCurrentDir() + "\\Shaders\\GenerateNormals.comp";
    AddShader(normalMapGenerationCS, GenerateNormalsShader);

    updateNormalMapCS = "UpdateNormalsCS";
    std::unordered_map<SHADER_TYPES, std::string> UpdateNormalsShader;
    UpdateNormalsShader[SHADER_TYPES::COMPUT_SHADER] = GetCurrentDir() + "\\Shaders\\UpdateNormals.comp";
    AddShader(updateNormalMapCS, UpdateNormalsShader);

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
    normalMap = "normalMap";

    bool HDR = true;
    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory, HDR);
    LoadTexture(wallDiffuseMap, "wall.jpg", textureDirectory, HDR);
    LoadTexture(grassTexture, "grass.png", textureDirectory, HDR);
    LoadTexture(transparentWindowTexture, "blending_transparent_window.png", textureDirectory, HDR);
    //LoadTexture(heightMap, "BOTW_HeightMap.png", textureDirectory, HDR);
    //LoadTexture(normalMap, "BOTW_NormalMap.png", textureDirectory, false);

    // Generated unpreturbed height map
    terrainMeshWidth = 3200;
    terrainMeshHeight = 3200;
    heightMapWidth = 3200;
    heightMapHeight = 3200;
    terrainChunkSize = 16;
    terrainScale = 10.0f;
    heightScale = 100.0f;
    waterLevel = 5.0f;
    terrainMinHeight = 0.0f;
    terrainMaxHeight = heightScale;
    heightMapDirty = false;
    brushHighlightActive = true;
    brushRadius = 100;
    float lacunarity = 2.0f;
    float persistence = 0.5f;
    int octaves = 2;
    generatedHeightMap = std::make_shared<std::vector<float>>(heightMapWidth * heightMapHeight, 0);
    GenerateTerrainHeightMap(generatedHeightMap, heightMapWidth, heightMapHeight, lacunarity, persistence, octaves);

    // Generate Voronoi map
    generatedVoronoiMap = std::make_shared<std::vector<float>>(heightMapWidth * heightMapHeight, 0);
    GenerateVoroniMap(generatedVoronoiMap, heightMapWidth, heightMapHeight);

    //MergeHeightMaps(generatedHeightMap, generatedVoronoiMap, heightMapWidth, heightMapHeight);

    int nComponents = 1;
    HDR = false;
    bool generateMipMaps = false;
    LoadTextureRaw(customHeightMap, generatedHeightMap->data(), heightMapWidth, heightMapHeight, nComponents, HDR, generateMipMaps);
    
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
    //HydrolicErosion();

    auto heightRange = std::minmax_element(generatedHeightMap->begin(), generatedHeightMap->end());
    terrainMinHeight = *heightRange.first;
    terrainMaxHeight = *heightRange.second;

    // The height map is already loaded as a texture customHeightMap for the compute shader
    // Load the empty normal map
    calculatedNormalMap = std::make_shared<std::vector<glm::vec4>>(heightMapWidth * heightMapHeight, glm::vec4(0));
    customNormalMap = "Generated Normal Map";
    LoadBuffer(customNormalMap, calculatedNormalMap, (heightMapWidth* heightMapHeight) * sizeof(glm::vec4));

    // Generate the normal map using a compute shader
    GenerateNormals();

    // Add/Load Models
    //AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);
    AddPresetMesh("skyDome", DEFAULT_MESHES::SPHERE);

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

bool DemoTestScene::RayPlaneXZ(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float planeY, glm::vec3& hit)
{
    if (std::abs(rayDir.y) < 0.0001f)
        return false;

    float t = (planeY - rayOrigin.y) / rayDir.y;

    if (t < 0.0f)
        return false;

    hit = rayOrigin + rayDir * t;
    return true;
}

bool DemoTestScene::SampleTerrainHeight(glm::vec2 worldXZ, float& outHeight)
{
    float originX = -float(terrainMeshWidth - 1) * 0.5f * terrainScale;
    float originZ = -float(terrainMeshHeight - 1) * 0.5f * terrainScale;

    float u = (worldXZ.x - originX) / (float(terrainMeshWidth - 1) * terrainScale);
    float v = (worldXZ.y - originZ) / (float(terrainMeshHeight - 1) * terrainScale);

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
        return false;

    float px = u * float(heightMapWidth - 1);
    float py = v * float(heightMapHeight - 1);

    int x0 = int(floor(px));
    int y0 = int(floor(py));
    int x1 = std::min(x0 + 1, int(heightMapWidth - 1));
    int y1 = std::min(y0 + 1, int(heightMapHeight - 1));

    float tx = px - float(x0);
    float ty = py - float(y0);

    float h00 = (*generatedHeightMap)[x0 + y0 * heightMapWidth];
    float h10 = (*generatedHeightMap)[x1 + y0 * heightMapWidth];
    float h01 = (*generatedHeightMap)[x0 + y1 * heightMapWidth];
    float h11 = (*generatedHeightMap)[x1 + y1 * heightMapWidth];

    float h0 = glm::mix(h00, h10, tx);
    float h1 = glm::mix(h01, h11, tx);

    outHeight = glm::mix(h0, h1, ty);
    return true;
}

bool DemoTestScene::RayMarchTerrain(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float maxDistance, float stepSize, glm::vec3& hit)
{
    glm::vec3 previousPoint = rayOrigin;
    float previousTerrainHeight = 0.0f;

    if (!SampleTerrainHeight(glm::vec2(previousPoint.x, previousPoint.z), previousTerrainHeight))
    {
        previousTerrainHeight = -999999.0f;
    }

    float previousDiff = previousPoint.y - previousTerrainHeight;

    for (float t = stepSize; t <= maxDistance; t += stepSize)
    {
        glm::vec3 currentPoint = rayOrigin + rayDir * t;

        float terrainHeight;
        if (!SampleTerrainHeight(glm::vec2(currentPoint.x, currentPoint.z), terrainHeight))
            continue;

        float currentDiff = currentPoint.y - terrainHeight;

        if (previousDiff > 0.0f && currentDiff <= 0.0f)
        {
            glm::vec3 a = previousPoint;
            glm::vec3 b = currentPoint;

            // Binary refine so the brush does not jitter/chunk along the ray.
            for (int i = 0; i < 8; ++i)
            {
                glm::vec3 mid = (a + b) * 0.5f;

                float midHeight;
                SampleTerrainHeight(glm::vec2(mid.x, mid.z), midHeight);

                if (mid.y > midHeight)
                    a = mid;
                else
                    b = mid;
            }

            hit = (a + b) * 0.5f;
            return true;
        }

        previousPoint = currentPoint;
        previousDiff = currentDiff;
    }

    return false;
}

void DemoTestScene::ModifyHeightmap()
{
    if (brushHighlightActive == true)
    {
        glm::vec3 hit;
        glm::vec3 rayOrigin = GetCamera("MainCamera")->Position;
        glm::vec3 rayDir = glm::normalize(GetCamera("MainCamera")->Front);
        float strength = 2;

        if (RayMarchTerrain(rayOrigin, rayDir, 10000.0f, 4.0f, hit))
        {
            brushCenterXZ = glm::vec2(hit.x, hit.z);
            ApplyHeightBrush(brushCenterXZ, brushRadius, strength);
        }
    }
}

float DemoTestScene::FractalNoise(float x, float y, int octaves, float frequency, uint32_t seed)
{
    const siv::PerlinNoise perlin{ seed };

    float value = 0.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i)
    {
        value += float(perlin.noise2D_01(x * frequency, y * frequency)) * amplitude;
        maxValue += amplitude;

        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return value / maxValue; // 0..1
}

float DemoTestScene::RidgeNoise(float x, float y, uint32_t seed)
{
    float n = FractalNoise(x, y, 5, 0.018f, seed);
    n = n * 2.0f - 1.0f;       // -1..1
    n = 1.0f - std::abs(n);    // ridges
    return n * n;              // sharpen
}

float DemoTestScene::EvaluateBrushShape(glm::vec2 localPx, float distancePx, float radiusPx)
{
    float t = glm::clamp(distancePx / radiusPx, 0.0f, 1.0f);
    float falloff = BrushFalloff(distancePx, radiusPx, 0.45f);

    uint32_t seed = 68465164u;

    switch (activeBrushType)
    {
    case TerrainBrushType::SmoothHill:
        return falloff;

    case TerrainBrushType::SmoothPit:
        return -falloff;

    case TerrainBrushType::Noise:
    {
        float n = FractalNoise(localPx.x, localPx.y, 4, 0.025f, seed);
        n = n * 2.0f - 1.0f;
        return falloff * n;
    }

    case TerrainBrushType::RidgedMountain:
    {
        float cone = glm::pow(1.0f - t, 2.2f);
        float ridges = RidgeNoise(localPx.x, localPx.y, seed);
        return cone * (0.75f + ridges * 0.65f);
    }

    case TerrainBrushType::Volcano:
    {
        float mountain = glm::pow(1.0f - t, 2.0f);
        float crater = glm::smoothstep(0.0f, 0.18f, t);
        return falloff * mountain * crater;
    }

    case TerrainBrushType::Canyon:
    {
        float centerCut = 1.0f - glm::smoothstep(0.0f, 0.22f, std::abs(localPx.x) / radiusPx);
        float wallLift = glm::smoothstep(0.18f, 0.45f, std::abs(localPx.x) / radiusPx)
            * (1.0f - glm::smoothstep(0.45f, 1.0f, std::abs(localPx.x) / radiusPx));

        return falloff * (-centerCut * 1.2f + wallLift * 0.45f);
    }

    case TerrainBrushType::Plateau:
    {
        float flatTop = 1.0f - glm::smoothstep(0.52f, 0.9f, t);
        return flatTop;
    }

    case TerrainBrushType::Mesa:
    {
        float top = 1.0f - glm::smoothstep(0.48f, 0.58f, t);
        float edgeRoughness = RidgeNoise(localPx.x, localPx.y, seed) * 0.2f;
        return top + edgeRoughness * falloff;
    }
    }

    return falloff;
}

void DemoTestScene::ApplyHeightBrush(glm::vec2 worldXZ, float radiusWorld, float strength)
{
    float terrainOriginX = -float(terrainMeshWidth - 1) * 0.5f * terrainScale;
    float terrainOriginZ = -float(terrainMeshHeight - 1) * 0.5f * terrainScale;

    float u = (worldXZ.x - terrainOriginX) / (float(terrainMeshWidth - 1) * terrainScale);
    float v = (worldXZ.y - terrainOriginZ) / (float(terrainMeshHeight - 1) * terrainScale);

    glm::vec2 centerPx(
        u * float(heightMapWidth - 1),
        v * float(heightMapHeight - 1)
    );

    float radiusPx = radiusWorld / terrainScale;

    minX = glm::max(0, int(centerPx.x - radiusPx));
    maxX = glm::min(int(heightMapWidth - 1), int(centerPx.x + radiusPx));
    minY = glm::max(0, int(centerPx.y - radiusPx));
    maxY = glm::min(int(heightMapHeight - 1), int(centerPx.y + radiusPx));

    uint32_t currentBrushSeed = 68465164;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            glm::vec2 p(x, y);
            glm::vec2 localPx = p - centerPx;

            float d = glm::length(localPx);

            if (d > radiusPx)
                continue;

            float shape = EvaluateBrushShape(localPx, d, radiusPx);

            size_t idx = x + y * heightMapWidth;
            (*generatedHeightMap)[idx] += strength * shape;
        }
    }

    heightMapDirty = true;
}

float DemoTestScene::BrushFalloff(float distance, float radius, float softness)
{
    float t = glm::clamp(distance / radius, 0.0f, 1.0f);

    switch (activeFalloffType)
    {
        case BrushFalloffType::Linear:
            return 1.0f - t;

        case BrushFalloffType::Smooth:
            return 1.0f - SmoothStepCPU(0.0f, 1.0f, t);

        case BrushFalloffType::Sharp:
        {
            float smooth = 1.0f - SmoothStepCPU(0.0f, 1.0f, t);
            return smooth * smooth;
        }
    }

    //Default linear brush
    return 1.0f - t;
}

void DemoTestScene::UpdateGPUHightmap()
{
    if (heightMapDirty == true)
    {
        glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));

        for (int y = minY; y <= maxY; ++y)
        {
            const float* row = generatedHeightMap->data() + y * heightMapWidth + minX;

            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                minX,
                y,
                maxX - minX + 1,
                1,
                GL_RED,
                GL_FLOAT,
                row
            );
        }
        heightMapDirty = false;
    }
}

void DemoTestScene::MergeHeightMaps(std::shared_ptr<std::vector<float>> perlinFBMNoise, std::shared_ptr<std::vector<float>> voronoiNoise, size_t mapWidth, size_t mapHeight)
{
#ifdef _DEBUG
    Image image{ mapWidth, mapHeight };
#endif
    float mergeStart = 0.56f;
    float mergeFull = 0.82f;

    float minHeight = FLT_MAX;
    float maxHeight = -FLT_MAX;

    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            int idx = x + y * mapWidth;
            float perlinHeight = (*perlinFBMNoise)[idx];
            float mergeMask = SmoothStepCPU(mergeStart, mergeFull, perlinHeight);
            float voronoiValue = (*voronoiNoise)[idx];
            float voronoiHeight = glm::pow(glm::clamp(voronoiValue, 0.0f, 1.0f), 1.65f);
            float mergedNoiseHeight = glm::mix(perlinHeight, glm::max(perlinHeight, voronoiHeight), mergeMask * 0.45f);

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

            minHeight = std::min(minHeight, mergedNoiseHeight);
            maxHeight = std::max(maxHeight, mergedNoiseHeight);

            (*perlinFBMNoise)[idx] = mergedNoiseHeight;// * heightScale;
        }
    }

    float range = maxHeight - minHeight;

    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            int idx = x + y * mapWidth;

            float normalizedHeight = ((*perlinFBMNoise)[idx] - minHeight) / range;
            float finalHeight = normalizedHeight * heightScale;

            (*perlinFBMNoise)[idx] = finalHeight;
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

void DemoTestScene::GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float lacunarity, float persistence, int octaves)
{

#ifdef _DEBUG
    Image image{ mapWidth, mapHeight };
#endif
    std::uint32_t seed = 231842352;

    const siv::PerlinNoise perlin{ seed };
    const double baseFrequency = 12.0;
    const int octaveCount = std::max(1, octaves);
    const float persistenceValue = persistence;
    const float lacunarityValue = lacunarity;

    for (std::int32_t x = 0; x < mapWidth; ++x)
    {
        for (std::int32_t y = 0;y < mapHeight; ++y)
        {
            double frequency = baseFrequency;
            float amplitude = 1.0f;
            float amplitudeSum = 0.0f;
            float noiseValue = 0.0f;

            for (int octave = 0; octave < octaveCount; ++octave)
            {
                const double sampleX = (static_cast<double>(x) / mapWidth) * frequency;
                const double sampleY = (static_cast<double>(y) / mapHeight) * frequency;
                noiseValue += static_cast<float>(perlin.noise2D(sampleX, sampleY)) * amplitude;
                amplitudeSum += amplitude;
                amplitude *= persistenceValue;
                frequency *= lacunarityValue;
            }

            //noiseValue = (noiseValue / amplitudeSum) * 0.5f + 0.5f;
            //noiseValue = glm::clamp(noiseValue, 0.0f, 1.0f);
            
#ifdef _DEBUG
            const RGB color(noiseValue);
            image.set(x, y, color);
#endif
            float worldHeight = noiseValue * heightScale;
            float finalNoise = waterLevel + (worldHeight - waterLevel) * terrainScale;
            (*heightMap)[x + y * mapWidth] = finalNoise;
        }
    }

#ifdef _DEBUG
    std::stringstream ss;
    ss << "Heightmap_Perlin4Octaves_" << seed << ".bmp";

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

void DemoTestScene::DeltaTime(float _deltaTime)
{
    deltaTime = _deltaTime;

    if (dayNightCycle == true)
    {
        accTime += _deltaTime;
        float dayDurationSeconds = 240.0f; // Full day cycle every 2 minutes
        timeOfDay01 = std::fmod(accTime, dayDurationSeconds) / dayDurationSeconds;
    }
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
            d.x = randomInRange(0.0f, (float)heightMapWidth - 1.0f);
            d.y = randomInRange(0.0f, (float)heightMapHeight - 1.0f);
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

                if (d.x < 1 || d.x > heightMapWidth - 2 || d.y < 1 || d.y > heightMapHeight - 2)
                    break;

                // Calculate height and gradient using bilinear interpolation for smoothness
                float h00 = (*generatedHeightMap)[iy * heightMapWidth + ix];
                float h10 = (*generatedHeightMap)[iy * heightMapWidth + (ix + 1)];
                float h01 = (*generatedHeightMap)[(iy + 1) * heightMapWidth + ix];
                float h11 = (*generatedHeightMap)[(iy + 1) * heightMapWidth + (ix + 1)];

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

                if (d.x < 0 || d.x >= heightMapWidth - 1 || d.y < 0 || d.y >= heightMapHeight - 1)
                    break;

                // Get new height at new position using bilinear sampling
                int nix = (int)d.x;
                int niy = (int)d.y;
                float nu = d.x - nix;
                float nv = d.y - niy;
                float nh00 = (*generatedHeightMap)[niy * heightMapWidth + nix];
                float nh10 = (*generatedHeightMap)[niy * heightMapWidth + (nix + 1)];
                float nh01 = (*generatedHeightMap)[(niy + 1) * heightMapWidth + nix];
                float nh11 = (*generatedHeightMap)[(niy + 1) * heightMapWidth + (nix + 1)];
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

                    (*generatedHeightMap)[iy * heightMapWidth + ix] += amountToDeposit * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * heightMapWidth + (ix + 1)] += amountToDeposit * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + ix] += amountToDeposit * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + (ix + 1)] += amountToDeposit * u * v;
                    d.sediment -= amountToDeposit;
                }
                else if (d.sediment > capacity) {
                    // Oversaturated: standard deposition using the tuned depositSpeed
                    float amountToDeposit = (d.sediment - capacity) * depositSpeed;
                    amountToDeposit *= erosionMultiplier;

                    (*generatedHeightMap)[iy * heightMapWidth + ix] += amountToDeposit * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * heightMapWidth + (ix + 1)] += amountToDeposit * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + ix] += amountToDeposit * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + (ix + 1)] += amountToDeposit * u * v;
                    d.sediment -= amountToDeposit;
                }
                else {
                    // Erode - distribute across 4 neighbors at old position for smoothness
                    float amountToErode = std::min((capacity - d.sediment) * erodeSpeed, -deltaH);
                    amountToErode *= erosionMultiplier;

                    (*generatedHeightMap)[iy * heightMapWidth + ix] -= amountToErode * (1 - u) * (1 - v);
                    (*generatedHeightMap)[iy * heightMapWidth + (ix + 1)] -= amountToErode * u * (1 - v);
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + ix] -= amountToErode * (1 - u) * v;
                    (*generatedHeightMap)[(iy + 1) * heightMapWidth + (ix + 1)] -= amountToErode * u * v;
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
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, heightMapWidth, heightMapHeight, GL_RED, GL_FLOAT, generatedHeightMap->data());
        return;
    }
}

void DemoTestScene::GenerateNormals()
{
    UseShaderProgram(normalMapGenerationCS);

    unsigned int computeProgram = GetShaderProgramID(normalMapGenerationCS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
    glUniform1i(glGetUniformLocation(computeProgram, "heightMap"), 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetBufferID(customNormalMap)); // Binding 1

    // The height and normal buffers are heightmap-sized, independent of mesh density.
    int gridDimX = static_cast<int>(((heightMapWidth * heightMapHeight) + 63) / 64);
    float terrainWorldSizeX = static_cast<float>(terrainMeshWidth - 1) * terrainScale;
    float terrainWorldSizeZ = static_cast<float>(terrainMeshHeight - 1) * terrainScale;
    float normalSpacingX = terrainWorldSizeX / static_cast<float>(heightMapWidth - 1);
    float normalSpacingZ = terrainWorldSizeZ / static_cast<float>(heightMapHeight - 1);

    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimX"), static_cast<int>(heightMapWidth));
    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimY"), static_cast<int>(heightMapHeight));
    glUniform1f(glGetUniformLocation(computeProgram, "spacingX"), normalSpacingX);
    glUniform1f(glGetUniformLocation(computeProgram, "spacingZ"), normalSpacingZ);

    glDispatchCompute(gridDimX, 1, 1);

    // ensure writes are visible to subsequent reads/copies
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);


#ifdef _DEBUG
    // Read back the normal map and dump it into an image
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GetBufferID(customNormalMap));

    GLsizeiptr size = heightMapWidth * heightMapHeight * sizeof(glm::vec4);
    void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, GL_MAP_READ_BIT);

    if (ptr) {
        glm::vec4* normalsBuffer = reinterpret_cast<glm::vec4*>(ptr);

        Image image{ heightMapWidth, heightMapHeight };

        for (std::int32_t x = 0; x < heightMapWidth; ++x)
        {
            for (std::int32_t y = 0; y < heightMapHeight; ++y)
            {
                glm::vec4 temp = normalsBuffer[x + y * heightMapWidth];
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

void DemoTestScene::UpdateNormals()
{
    // Wait for the texture sub
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    // region thatv was marked as dirty for normal update
    int normalMinX = std::max(0, minX - 1);
    int normalMinY = std::max(0, minY - 1);
    int normalMaxX = std::min(int(heightMapWidth - 1), maxX + 1);
    int normalMaxY = std::min(int(heightMapHeight - 1), maxY + 1);

    int normalWidth = normalMaxX - normalMinX + 1;
    int normalHeight = normalMaxY - normalMinY + 1;
    
    // Update the normals for the terrain region that was updated
    UseShaderProgram(updateNormalMapCS);

    unsigned int computeProgram = GetShaderProgramID(updateNormalMapCS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
    glUniform1i(glGetUniformLocation(computeProgram, "heightMap"), 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetBufferID(customNormalMap));

    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimX"), static_cast<int>(heightMapWidth));
    glUniform1i(glGetUniformLocation(computeProgram, "terrainDimY"), static_cast<int>(heightMapHeight));

    glUniform2i(glGetUniformLocation(computeProgram, "dirtyMin"), normalMinX, normalMinY);
    glUniform2i(glGetUniformLocation(computeProgram, "dirtySize"), normalWidth, normalHeight);

    float terrainWorldSizeX = static_cast<float>(terrainMeshWidth - 1) * terrainScale;
    float terrainWorldSizeZ = static_cast<float>(terrainMeshHeight - 1) * terrainScale;
    float normalSpacingX = terrainWorldSizeX / static_cast<float>(heightMapWidth - 1);
    float normalSpacingZ = terrainWorldSizeZ / static_cast<float>(heightMapHeight - 1);

    glUniform1f(glGetUniformLocation(computeProgram, "spacingX"), normalSpacingX);
    glUniform1f(glGetUniformLocation(computeProgram, "spacingZ"), normalSpacingZ);

    GLuint groupsX = GLuint((normalWidth + 15) / 16);
    GLuint groupsY = GLuint((normalHeight + 15) / 16);

    glDispatchCompute(groupsX, groupsY, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void DemoTestScene::UpdateDirtyChunkBounds()
{
    int dirtyMeshMinX = int((float(minX) / float(heightMapWidth - 1)) * float(terrainMeshWidth - 1));
    int dirtyMeshMaxX = int((float(maxX) / float(heightMapWidth - 1)) * float(terrainMeshWidth - 1));
    int dirtyMeshMinY = int((float(minY) / float(heightMapHeight - 1)) * float(terrainMeshHeight - 1));
    int dirtyMeshMaxY = int((float(maxY) / float(heightMapHeight - 1)) * float(terrainMeshHeight - 1));

    int chunkMinX = dirtyMeshMinX / int(terrainChunkSize);
    int chunkMaxX = dirtyMeshMaxX / int(terrainChunkSize);
    int chunkMinY = dirtyMeshMinY / int(terrainChunkSize);
    int chunkMaxY = dirtyMeshMaxY / int(terrainChunkSize);

    size_t maxMeshX = terrainMeshWidth - 1;
    size_t maxMeshY = terrainMeshHeight - 1;
    size_t chunkCountX = (maxMeshX + terrainChunkSize - 1) / terrainChunkSize;

    for (int cy = chunkMinY; cy <= chunkMaxY; ++cy)
    {
        for (int cx = chunkMinX; cx <= chunkMaxX; ++cx)
        {
            size_t chunkIndex = size_t(cx) + size_t(cy) * chunkCountX;

            if (chunkIndex >= terrainChunks.size())
                continue;

            TerrainChunk& chunk = terrainChunks[chunkIndex];

            size_t startX = size_t(cx) * terrainChunkSize;
            size_t startY = size_t(cy) * terrainChunkSize;
            size_t endX = std::min(startX + terrainChunkSize, maxMeshX);
            size_t endY = std::min(startY + terrainChunkSize, maxMeshY);

            float chunkMinHeight = std::numeric_limits<float>::max();
            float chunkMaxHeight = std::numeric_limits<float>::lowest();

            for (size_t meshY = startY; meshY <= endY; ++meshY)
            {
                for (size_t meshX = startX; meshX <= endX; ++meshX)
                {
                    size_t hMapX = size_t((float(meshX) / float(terrainMeshWidth - 1)) * float(heightMapWidth - 1));
                    size_t hMapY = size_t((float(meshY) / float(terrainMeshHeight - 1)) * float(heightMapHeight - 1));

                    float h = (*generatedHeightMap)[hMapX + hMapY * heightMapWidth];

                    chunkMinHeight = std::min(chunkMinHeight, h);
                    chunkMaxHeight = std::max(chunkMaxHeight, h);
                }
            }

            //That protects you from tiny mismatches between CPU sampling, tessellation interpolation, and GPU height sampling.
            float boundsPadding = 5.0f;
            chunk.minBounds.y = chunkMinHeight - boundsPadding;
            chunk.maxBounds.y = chunkMaxHeight + boundsPadding;
        }
    }
}

void DemoTestScene::BuildTerrainChunks()
{
    terrainChunks.clear();

    const float terrainOriginX = -static_cast<float>(terrainMeshWidth - 1) * 0.5f * terrainScale;
    const float terrainOriginZ = -static_cast<float>(terrainMeshHeight - 1) * 0.5f * terrainScale;
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

                    // Scale mesh coordinates to heightmap resolution
                    size_t hMapX = static_cast<size_t>((static_cast<float>(sampleX) / (terrainMeshWidth - 1)) * (heightMapWidth - 1));
                    size_t hMapY = static_cast<size_t>((static_cast<float>(sampleY) / (terrainMeshHeight - 1)) * (heightMapHeight - 1));
                    float height = (*generatedHeightMap)[hMapX + hMapY * heightMapWidth];

                    chunkMinHeight = std::min(chunkMinHeight, height);
                    chunkMaxHeight = std::max(chunkMaxHeight, height);

                    Vertex vertex;
                    vertex.Position = glm::vec3(terrainOriginX + static_cast<float>(sampleX) * terrainScale, 0.0f, terrainOriginZ + static_cast<float>(sampleY) * terrainScale);
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
            chunk.minBounds = glm::vec3(terrainOriginX + static_cast<float>(startX) * terrainScale, chunkMinHeight, terrainOriginZ + static_cast<float>(startY) * terrainScale);
            chunk.maxBounds = glm::vec3(terrainOriginX + static_cast<float>(endX) * terrainScale, chunkMaxHeight, terrainOriginZ + static_cast<float>(endY) * terrainScale);

            AddCustomMesh(chunk.meshName, chunkMesh);
            terrainChunks.push_back(chunk);
        }
    }
}

void DemoTestScene::DemoKeyPressed(uint16_t keyCode)
{
    if (keyCode == GLFW_KEY_H) brushHighlightActive = !brushHighlightActive;
    if (keyCode == GLFW_KEY_UP) brushRadius += 5;
    if (keyCode == GLFW_KEY_DOWN) brushRadius = glm::max(5.0f, (brushRadius - 5));

    // Set TerrainBrushType
    if (keyCode == GLFW_KEY_O)
    {
        int next = static_cast<int>(activeBrushType) + 1;
        next %= static_cast<int>(TerrainBrushType::Count);

        activeBrushType = static_cast<TerrainBrushType>(next);
    }

    // Switch BrushFalloffType type
    if (keyCode == GLFW_KEY_P)
    {
        int next = static_cast<int>(activeFalloffType) + 1;
        next %= static_cast<int>(BrushFalloffType::Count);

        activeFalloffType = static_cast<BrushFalloffType>(next);
    }

    if (keyCode == GLFW_KEY_C) dayNightCycle = !dayNightCycle;
}

void DemoTestScene::RenderScene(unsigned int deferredQuadFrameBuffer)
{
    // Update terrain before rendering
    if (leftMouseHeld == true)
    {
        ModifyHeightmap();
        UpdateGPUHightmap();
        UpdateNormals();
        UpdateDirtyChunkBounds();
    }

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Daynight cycle calculations
    float dayProgress = timeOfDay01 * 2.0f * 3.14159265f;

    // Create a circular orbit in a local vertical plane (rising at East, setting at West)
    glm::vec3 orbitPos = glm::vec3(std::cos(dayProgress), std::sin(dayProgress), 0.0f);

    // Tilt the orbital plane by 35 degrees around the East-West axis (X-axis). 
    // This simulates latitude, causing the sun to rise and set at an angle relative 
    // to the horizon and follow a lower, curved arc across the sky.
    glm::mat4 tiltMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 sunDirection = glm::vec3(tiltMatrix * glm::vec4(orbitPos, 0.0f));

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
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100000.0f);
    tessShaderProgram->setMat4("projection", projection);

    // Custom plane mesh
    glDisable(GL_CULL_FACE);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["customPlaneMeshPosition"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    tessShaderProgram->setMat4("model", model);
    tessShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
    tessShaderProgram->setInt("terrainDimX", static_cast<int>(heightMapWidth));
    tessShaderProgram->setInt("terrainDimY", static_cast<int>(heightMapHeight));
    tessShaderProgram->setFloat("terrainMinHeight", terrainMinHeight);
    tessShaderProgram->setFloat("terrainMaxHeight", terrainMaxHeight);
    tessShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

    tessShaderProgram->setVec3("sunDirection", sunDirection);

    tessShaderProgram->setBool("brushHighlightActive", brushHighlightActive);

    // Get the current terrain hit point for the brush center
    if (brushHighlightActive == true)
    {
        glm::vec3 rayOrigin = GetCamera("MainCamera")->Position;
        glm::vec3 rayDir = glm::normalize(GetCamera("MainCamera")->Front);
        glm::vec3 hit;
        RayMarchTerrain(rayOrigin, rayDir, 10000.0f, 4.0f, hit);
        brushCenterXZ = glm::vec2(hit.x, hit.z);
    }
    tessShaderProgram->setVec3("brushCenterWorld", glm::vec3(brushCenterXZ.x, 0.0f, brushCenterXZ.y));
    
    tessShaderProgram->setFloat("brushRadiusWorld", brushRadius);
    tessShaderProgram->setFloat("brushRingWidth", 4.0f);

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
    skyModel = glm::scale(skyModel, glm::vec3(500000.0f)); // Scale up so it surrounds the scene
    skyboxShaderProgram->setMat4("model", skyModel);

    // Use the same far plane as the terrain (10000.0) to ensure the skydome is not clipped
    skyboxShaderProgram->setMat4("projection", projection); 
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
    waterModel = glm::scale(waterModel, glm::vec3(static_cast<float>(terrainMeshWidth - 1) * terrainScale * 0.5f, 1.0f, static_cast<float>(terrainMeshHeight - 1) * terrainScale * 0.5f));
    waterShaderProgram->setMat4("model", waterModel);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    DrawMesh("plane", waterShaderProgramName);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}
