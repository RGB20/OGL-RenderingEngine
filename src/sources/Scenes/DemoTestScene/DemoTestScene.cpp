#include "headers/Scenes/DemoTestScene/DemoTestScene.h"
#include <map>
#include "headers/Mesh.h"

void DemoTestScene::SetupScene()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 8.0f, 0.0f)));

    blendingShaderProgramName = "blendingShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> blendingShaders;

    blendingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    blendingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(blendingShaderProgramName, blendingShaders);

    skyboxShaderProgramName = "skyboxShaderProgram";

    std::unordered_map<SHADER_TYPES, std::string> skyboxShaders;

    skyboxShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\skyboxVertexShader.vs";
    skyboxShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\skyboxFragmentShader.fs";
    AddShader(skyboxShaderProgramName, skyboxShaders);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    wallDiffuseMap = "wallDiffuseMap";
    grassTexture = "grassTexture";
    transparentWindowTexture = "transparentWindowTexture";
    skyboxTexture = "skyboxTexture";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory, true);
    LoadTexture(wallDiffuseMap, "wall.jpg", textureDirectory, true);
    LoadTexture(grassTexture, "grass.png", textureDirectory, true);
    LoadTexture(transparentWindowTexture, "blending_transparent_window.png", textureDirectory, true);

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
    GetShaderProgram(skyboxShaderProgramName)->setInt("skyboxTexture", 0);

    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Create a quad based mesh which is equivalent to the 2D plane
    std::shared_ptr<Mesh> customPlaneMesh = std::make_shared<Mesh>();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float terrainMeshWidth = 1600;
    float terrainMeshHeight = 1600;

    patchInfo = std::make_shared<PatchInfo>();
    patchInfo->resX = 10;
    patchInfo->resY = 10;
    patchInfo->patchPrimCount = PATCH_PRIM_TYPE::TRI_MESH;
    
    int stepSizeX = terrainMeshWidth / patchInfo->resX;
    int stepSizeY = terrainMeshHeight / patchInfo->resY;

    for (int z = 0; z < patchInfo->resY; ++z) {
        for (int x = 0; x < patchInfo->resX; ++x) {
            Vertex v;

            v.Position = glm::vec3((-terrainMeshWidth/2) + (x * stepSizeX), 0.0f, (- terrainMeshHeight/2)  + (z * stepSizeY));
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
            indices.push_back(t1_bottomRight);
            indices.push_back(t1_topLeft);

            indices.push_back(t2_bottomRight);
            indices.push_back(t2_topLeft);
            indices.push_back(t2_topRight);
        }
    }


    //int depthMapWidth = 10;
    //int depthMapHeight = 10;

    //for (int z = 0; z <= (patchInfo->resY-1); z++)
    //{
    //    for (int x = 0; x <= (patchInfo->resX-1); x++)
    //    {
    //        // We will loop over all the vertices going from the back
    //        Vertex vertex;
    //        float xPos = -depthMapWidth / 2 + depthMapWidth * x / (float)patchInfo->resX;
    //        float zPos = -depthMapHeight / 2 + depthMapHeight * z / (float)patchInfo->resX;
    //        vertex.Position = glm::vec3(xPos, 0.0f, zPos);

    //        float u = x / ((float)patchInfo->resX - 1);
    //        float v = z / ((float)patchInfo->resY - 1);
    //        vertex.TexCoords = glm::vec2(u, v);

    //       /* vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    //        vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    //        vertex.BiTangent = glm::normalize(glm::cross(vertex.Normal, vertex.Tangent));*/

    //        vertices.push_back(vertex);
    //    }
    //}

    //for (int x = 0; x < (patchInfo->resX - 1); x++)
    //{
    //    for (int y = 0; y < (patchInfo->resY - 1); y++)
    //    {
    //        unsigned int topLeft = x * patchInfo->resY + y;
    //        unsigned int topRight = topLeft + 1;
    //        unsigned int bottomLeft = topLeft + patchInfo->resY;
    //        unsigned int botttomRight = (bottomLeft) + 1;
    //        
    //        indices.push_back(topLeft);
    //        indices.push_back(topRight);
    //        indices.push_back(bottomLeft);
    //        indices.push_back(botttomRight);
    //    }
    //}

    //for (int i = 0; i < (vertices.size()); i + 4)
    //{
    //    glm::vec3 pos1 = vertices[i].Position;
    //    glm::vec3 pos2 = vertices[i+1].Position;
    //    glm::vec3 pos3 = vertices[i+2].Position;
    //    glm::vec3 pos4 = vertices[i+3].Position;

    //    glm::vec2 uv1 = vertices[i].TexCoords;
    //    glm::vec2 uv2 = vertices[i+1].TexCoords;
    //    glm::vec2 uv3 = vertices[i+2].TexCoords;
    //    glm::vec2 uv4 = vertices[i+3].TexCoords;
    //}

    customPlaneMesh->SetupMesh(vertices, indices, textures);

    AddCustomMesh("CustomPlane", customPlaneMesh);

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

    // Custom plane mesh
    glDisable(GL_CULL_FACE);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["customPlaneMeshPosition"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    objectShaderProgram->setMat4("model", model);
    DrawMesh("CustomPlane", blendingShaderProgramName, false, 0, patchInfo);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);

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