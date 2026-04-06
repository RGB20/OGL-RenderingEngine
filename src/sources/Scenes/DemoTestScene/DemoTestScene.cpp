#include "headers/Scenes/DemoTestScene/DemoTestScene.h"
#include <map>
#include "headers/Mesh.h"
#include <cmath>
#include <random>
#include <chrono>
#include <memory>
#include <headers/PerlinNoise.h>
#include "headers/PerlinNoiseLib.hpp"

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
    tessShaders[SHADER_TYPES::GEOMETRY_SHADER] = GetCurrentDir() + "\\shaders\\tessGeometryShader.gs";
    tessShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\tessFragmentShader.fs";
    AddShader(tessShaderProgramName, tessShaders);

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
    terrainMeshWidth = 1600;
    terrainMeshHeight = 1600;// 4096;
    float heightScale = 0.3;
    float lacunarify = 2.0;
    float persistance = 0.5;
    int octaves = 1;
    generatedHeightMap = std::make_shared<std::vector<float>>(terrainMeshWidth * terrainMeshHeight, 0);
    GenerateTerrainHeightMap(generatedHeightMap, terrainMeshWidth, terrainMeshHeight, heightScale, lacunarify, persistance, octaves);

    // Generate Voronoi map
    generatedVoronoiMap = std::make_shared<std::vector<float>>(terrainMeshWidth * terrainMeshHeight, 0);
    GenerateVoroniMap(generatedVoronoiMap, terrainMeshWidth, terrainMeshHeight);

    MergeHeightMaps(generatedHeightMap, generatedVoronoiMap, terrainMeshWidth, terrainMeshHeight);

    int nComponents = 1;
    HDR = false;
    LoadTextureRaw(customHeightMap, generatedHeightMap->data(), terrainMeshWidth, terrainMeshHeight, nComponents, HDR);
    
    erosionSimIterations = 10;

    //Loading a pre-calculated height map
    //std::string textureDirectory_custom = GetCurrentDir();
    //LoadTexture(customHeightMap, "f16o4_51684521.bmp", textureDirectory_custom, HDR);
    //LoadTexture(customHeightMap, "BOTW_HeightMap.png", textureDirectory, HDR);

    std::string skyboxtextureDirectory = GetCurrentDir() + "\\textures\\skyboxTextures\\OceanAndSky\\";

    std::vector<std::string> cubemapFaces;
    cubemapFaces.push_back("right.jpg");
    cubemapFaces.push_back("left.jpg");
    cubemapFaces.push_back("top.jpg");
    cubemapFaces.push_back("bottom.jpg");
    cubemapFaces.push_back("front.jpg");
    cubemapFaces.push_back("back.jpg");
    LoadCubeMapTexture("skyboxCubeMap", cubemapFaces, skyboxtextureDirectory);

    GetShaderProgram(blendingShaderProgramName)->setInt("material.diffuse", 0);
    GetShaderProgram(skyboxShaderProgramName)->setInt(skyboxTexture, 0);
    GetShaderProgram(tessShaderProgramName)->setInt(customHeightMap, 0);
    //GetShaderProgram(tessShaderProgramName)->setInt("normalMap", 1);

    //UseShaderProgram(tessNormalVisualizationShaderProgramName);
    //GetShaderProgram(tessNormalVisualizationShaderProgramName)->setInt("heightMap", 0);
    //GetShaderProgram(tessNormalVisualizationShaderProgramName)->setInt("normalMap", 1);


    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Create a quad based mesh which is equivalent to the 2D plane
    std::shared_ptr<Mesh> customPlaneMesh = std::make_shared<Mesh>();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    patchInfo = std::make_shared<PatchInfo>();
    patchInfo->resX = 256;
    patchInfo->resY = 256;
    patchInfo->patchPrimCount = PATCH_PRIM_TYPE::TRI_MESH;
    
    int stepSizeX = terrainMeshWidth / patchInfo->resX;
    int stepSizeY = terrainMeshHeight / patchInfo->resY;

    for (int z = 0; z < patchInfo->resY; ++z) {
        for (int x = 0; x < patchInfo->resX; ++x) {
            Vertex v;

            v.Position = glm::vec3((-(float)terrainMeshWidth/2) + (x * stepSizeX), 0.0f, (-(float)terrainMeshHeight/2)  + (z * stepSizeY));
            v.TexCoords = glm::vec2(float(x) / (patchInfo->resX - 1), float(z) / (patchInfo->resY - 1));

            vertices.push_back(v);
        }
    }

    for (int y = 0; y < patchInfo->resY - 1; ++y) {
        for (int x = 0; x < patchInfo->resX - 1; ++x) 
        {
            // Tri 1
            unsigned int t1_bottomLeft = x + y * patchInfo->resX;
            unsigned int t1_bottomRight = t1_bottomLeft + 1;
            unsigned int t1_topLeft = t1_bottomLeft + patchInfo->resY;

            // Tri 2
            unsigned int t2_bottomRight = t1_bottomRight;
            unsigned int t2_topLeft = t1_topLeft;
            unsigned int t2_topRight = t2_topLeft + 1;

            indices.push_back(t1_bottomLeft);
            indices.push_back(t1_topLeft);
            indices.push_back(t1_bottomRight);

            indices.push_back(t2_bottomRight);
            indices.push_back(t2_topLeft);
            indices.push_back(t2_topRight);
        }
    }

    customPlaneMesh->SetupMesh(vertices, indices, textures);

    AddCustomMesh("terrain", customPlaneMesh);

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

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["cubePositions"] = cubePositions;
    sceneAttributes["windowPanelPositions"] = windowPanelPositions;
    sceneAttributes["customPlaneMeshPosition"] = customPlaneMeshPosition;

    accTime = 0;
}

void DemoTestScene::MergeHeightMaps(std::shared_ptr<std::vector<float>> perlinFBMNoise, std::shared_ptr<std::vector<float>> voronoiNoise, size_t mapWidth, size_t mapHeight)
{
#ifdef _DEBUG
    Image image{ mapWidth, mapHeight };
#endif
    // 2/3 perlin noise, 1/3 voronoi noise
    float voronoiContribution = 0.3f;
    float perlinContribution = 0.7f;
    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            float mergedNoiseHeight = (*perlinFBMNoise)[y + x * mapWidth] * perlinContribution + (*voronoiNoise)[y + x * mapWidth] * voronoiContribution;

#ifdef _DEBUG
            const RGB color(mergedNoiseHeight);
            image.set(x, y, color);
#endif
            (*perlinFBMNoise)[y + x * mapWidth] = mergedNoiseHeight * 123.0f;
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
            int idx = y * mapWidth + x;
            float minDistance = closestSeedIndex(x, y, seedDimensionXY, seeds, mapWidth, mapHeight);
            (*voronoiMap)[y + x * mapWidth] = 1.0f - minDistance;
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

void DemoTestScene::GenerateTerrainHeightMap(std::shared_ptr<std::vector<float>> heightMap, size_t mapWidth, size_t mapHeight, float heightScale, float lacunarity, float persistance, int octaves)
{

#ifdef _DEBUG
    Image image{ mapWidth, mapWidth };
#endif
    double frequency = 10;

    //std::cout << "double frequency = ";
    //std::cin >> frequency;
    //frequency = std::clamp(frequency, 0.1, 64.0);

    std::int32_t octaves_in = 9;
    //std::cout << "int32 octaves    = ";
    //std::cin >> octaves_in;
    //octaves_in = std::clamp(octaves_in, 1, 16);

    std::uint32_t seed = 231842352;
    //std::cout << "uint32 seed      = ";
    //std::cin >> seed;

    const siv::PerlinNoise perlin{ seed };
    const double fx = (frequency / mapWidth);
    const double fy = (frequency / mapHeight);

    for (std::int32_t y = 0; y < mapHeight; ++y)
    {
        for (std::int32_t x = 0; x < mapWidth; ++x)
        {
            float noiseValue = perlin.octave2D_01((x * fx), (y * fy), octaves_in);
            // Modulate the noise
#ifdef _DEBUG
            const RGB color(noiseValue);
            image.set(x, y, color);
#endif
            (*heightMap)[y + x * mapWidth] = noiseValue;
        }
    }

#ifdef _DEBUG
    std::stringstream ss;
    ss << "Heightmap_" << 'f' << frequency << 'o' << octaves_in << '_' << seed << ".bmp";

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
    angleAroundCenter = std::fmod(accTime, 360.0f);
}

void DemoTestScene::HydrolicErosion()
{
    if (erosionSimIterations > 0) {
        erosionSimIterations -= 1;

        int maxErosionSteps = 1200;
        int numberOfDroplets = 1000000;

        for (int dropletNumber = 0; dropletNumber < numberOfDroplets; dropletNumber++)
        {
            Droplet d;
            d.x = randomInRange(0.0f, terrainMeshWidth);
            d.y = randomInRange(0.0f, terrainMeshHeight);

            for (int step = 0; step < maxErosionSteps; step++) {
                int ix = (int)d.x;
                int iy = (int)d.y;

                if (d.x < 1 || d.x > terrainMeshWidth - 2 || d.y < 1 || d.y > terrainMeshHeight - 2)
                    break;

                // Compute gradient using central differences
                float hL = (*generatedHeightMap)[iy * terrainMeshWidth + (ix - 1)]; // ix - 1][iy];
                float hR = (*generatedHeightMap)[iy * terrainMeshWidth + (ix + 1)]; // ix + 1][iy];
                float hD = (*generatedHeightMap)[(iy - 1) * terrainMeshWidth + ix]; //ix][iy - 1];
                float hU = (*generatedHeightMap)[(iy + 1) * terrainMeshWidth + ix]; //ix][iy + 1];

                float gradX = (hR - hL) * 0.5f;
                float gradY = (hU - hD) * 0.5f;

                // Update direction (inertia)
                d.dirX = d.dirX * 0.9f - gradX * 0.1f;
                d.dirY = d.dirY * 0.9f - gradY * 0.1f;

                // Normalize direction
                float len = sqrt(d.dirX * d.dirX + d.dirY * d.dirY);
                if (len != 0) {
                    d.dirX /= len;
                    d.dirY /= len;
                }

                // Move droplet
                d.x += d.dirX;
                d.y += d.dirY;

                // Height difference
                float newH = (*generatedHeightMap)[(int)d.y * terrainMeshWidth + (int)d.x]; // [(int)d.x] [(int)d.y] ;
                float oldH = (*generatedHeightMap)[iy * terrainMeshWidth + ix]; // [ix] [iy] ;
                float deltaH = newH - oldH;

                // Sediment capacity
                float capacity = std::max(-deltaH * d.speed * d.water * 4.0f, 0.0f);

                if (d.sediment > capacity) {
                    // Deposit
                    float deposit = (d.sediment - capacity) * 0.1f;
                    (*generatedHeightMap)[iy * terrainMeshWidth + ix] += deposit;
                    d.sediment -= deposit;
                }
                else {
                    // Erode
                    float erode = std::min((capacity - d.sediment) * 0.1f, oldH);
                    (*generatedHeightMap)[iy * terrainMeshWidth + ix] -= erode;
                    d.sediment += erode;
                }

                // Update droplet
                float slope = std::fabs(deltaH);
                d.speed = std::max(d.speed * d.speed + deltaH * 0.5f, 0.0f);
                d.speed = sqrt(d.speed);
                d.water *= 0.9f; // evaporation

                if (d.water < 0.01f)
                    break;
            }
        }

        glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, terrainMeshWidth, terrainMeshHeight, GL_RED, GL_FLOAT, generatedHeightMap->data());
        return;
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

    // Plane
    //glDisable(GL_CULL_FACE);
    //glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
    //model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    //objectShaderProgram->setMat4("model", model);
    //DrawMesh("plane", blendingShaderProgramName);
    //glBindVertexArray(0);
    //glEnable(GL_CULL_FACE);

    // --------------------------------------------------------------
    // Use TESS shader program to render the terrain
    UseShaderProgram(tessShaderProgramName);
    std::shared_ptr<Shader> tessShaderProgram = GetShaderProgram(tessShaderProgramName);
   
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID(customHeightMap));
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("normalMap"));

    // Set view matrix
    tessShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
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
    
    tessShaderProgram->setFloat("heightScale", 1);
    tessShaderProgram->setFloat("angleAroundCenter", 1.0f);// angleAroundCenter);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    DrawMesh("terrain", tessShaderProgramName, false, 0, patchInfo);
    
    HydrolicErosion();
    
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // --------------------------------------------------------------
    // Use tess NORMAL VISUALIZATION shader program to render the terrain
    //UseShaderProgram(tessNormalVisualizationShaderProgramName);
    //std::shared_ptr<Shader> tessNormalVisualizationShaderProgram = GetShaderProgram(tessNormalVisualizationShaderProgramName);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("heightMap"));
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("normalMap"));

    //// Set view matrix
    //tessNormalVisualizationShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    //// Projection
    //projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);
    //tessNormalVisualizationShaderProgram->setMat4("projection", projection);

    //// Custom plane mesh
    //glDisable(GL_CULL_FACE);
    //model = glm::mat4(1.0f);
    //model = glm::translate(model, sceneAttributes["customPlaneMeshPosition"][0]); // translate it down so it's at the center of the scene
    //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    ////model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    //tessNormalVisualizationShaderProgram->setMat4("model", model);
    //tessNormalVisualizationShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //DrawMesh("CustomPlane", tessNormalVisualizationShaderProgramName, false, 0, patchInfo);
    //glBindVertexArray(0);
    //glEnable(GL_CULL_FACE);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //// Render the Cubes
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    //{
    //    // Cube 1
    //    glm::mat4 model = glm::mat4(1.0f);
    //    model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
    //    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
    //    objectShaderProgram->setMat4("model", model);
    //    DrawMesh("cube", blendingShaderProgramName);
    //}
    //{
    //    // Cube 2
    //    glm::mat4 model = glm::mat4(1.0f);
    //    model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
    //    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
    //    objectShaderProgram->setMat4("model", model);
    //    DrawMesh("cube", blendingShaderProgramName);
    //}

    // Draw SKYBOX before the transparent meshes
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    UseShaderProgram(skyboxShaderProgramName);
    std::shared_ptr<Shader> skyboxShaderProgram = GetShaderProgram(skyboxShaderProgramName);

    skyboxShaderProgram->setMat4("view", glm::mat4(glm::mat3(GetCamera("MainCamera")->GetViewMatrix()))); // remove translation from the view matrix
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    skyboxShaderProgram->setMat4("projection", projection);

    // draw the skybox cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetTextureID("skyboxCubeMap"));
    DrawMesh("cube", skyboxShaderProgramName);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default

    // Draw Planes with transparent textures
    //UseShaderProgram(blendingShaderProgramName);
    ////std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(blendingShaderProgramName);

    //// VS stage Uniform inputs
    //// Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    //// Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    //// You can set the uniforms once or every frame
    //// View
    //objectShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    //// Projection
    //projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    //objectShaderProgram->setMat4("projection", projection);
    // 
    //// Set the material diffuse and specular maps
    //// texture1 - Material diffuse
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, GetTextureID("transparentWindowTexture"));

    //std::map<float, glm::vec3> sortedList;
    //for (unsigned int i = 0; i < sceneAttributes["windowPanelPositions"].size(); i++)
    //{
    //    float distance = glm::length(GetCamera("MainCamera")->Position - sceneAttributes["windowPanelPositions"][i]);
    //    sortedList[distance] = sceneAttributes["windowPanelPositions"][i];
    //}

    ////unsigned int listSize = sortedList.size() - 1;
    //for (std::map<float, glm::vec3>::reverse_iterator it = sortedList.rbegin(); it != sortedList.rend(); ++it)
    //{
    //    glm::mat4 model = glm::mat4(1.0f);
    //    model = glm::translate(model, it->second); // translate it down so it's at the center of the scene
    //    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
    //    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //    objectShaderProgram->setMat4("model", model);
    //    DrawMesh("plane", blendingShaderProgramName);
    //}
    //glEnable(GL_CULL_FACE);

    //// Sraw SKYBOX last
    //glDisable(GL_CULL_FACE);

    //glEnable(GL_CULL_FACE);


    glBindVertexArray(0);
}