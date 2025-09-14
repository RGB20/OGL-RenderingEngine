#include "headers/Scenes/NormalMappingTestScene/NormalMappingTestScene.h"

void NormalMappingTestScene::SetupScene()
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    normalMappingShaderProgramName = "normalMappingShaderProgram";
    lightShaderProgramName = "lightShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> normalMappingShaders;

    normalMappingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\normalMappingVertexShader.vs";
    normalMappingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\normalMappingFragmentShader.fs";
    AddShader(normalMappingShaderProgramName, normalMappingShaders);

    std::unordered_map<SHADER_TYPES, std::string> lightingShaders;

    lightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    lightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    AddShader(lightShaderProgramName, lightingShaders);

    // Load Textures
    std::string brickWallDiffuseTexMapFolder = GetCurrentDir() + "\\textures\\";
    std::string normalMapFolder = GetCurrentDir() + "\\textures\\";

    brickWallDiffuseMap = "brickWallDiffuseMap";
    normalMap = "normalMap";

    LoadTexture(brickWallDiffuseMap, "brickWall.jpg", brickWallDiffuseTexMapFolder);
    LoadTexture(normalMap, "brickwall_normalMap.jpg", normalMapFolder);

    UseShaderProgram(normalMappingShaderProgramName);
    GetShaderProgram(normalMappingShaderProgramName)->setInt("material.diffuse", 0);
    GetShaderProgram(normalMappingShaderProgramName)->setInt("material.normalMap", 1);

    // Add/Load Models
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(0.0f, -0.10f, 0.0f));

    // Directional Light direction
    std::vector<glm::vec3> directionalLightPosition;
    directionalLightPosition.push_back(glm::vec3(0.5f, 1.0f, 0.3f));

    std::vector<glm::vec3> lightColor;
    lightColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["directionalLightPosition"] = directionalLightPosition;
    sceneAttributes["lightColor"] = lightColor;
}

void NormalMappingTestScene::RenderScene(unsigned int deferredQuadFrameBuffer)
{
    // Render Normal Mapping Test Scene
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredQuadFrameBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glCullFace(GL_BACK);

    // Render Objects
    UseShaderProgram(normalMappingShaderProgramName);
    std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(normalMappingShaderProgramName);

    // VS stage Uniform inputs
    // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    // You can set the uniforms once or every frame
    // View
    objectShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    objectShaderProgram->setMat4("projection", projection);

    // FS stage Uniform inputs
    objectShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

    objectShaderProgram->setFloat("material.shininess", 32.0f);

    //DIRECTIONAL LIGHT
    objectShaderProgram->setVec3("lightPos", sceneAttributes["directionalLightPosition"][0]);

    // Planes VAO
    // Set the material diffuse and specular maps
    // texture1 - Material diffuse
    // texture2 - Normal map 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("brickWallDiffuseMap"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("normalMap"));

    // Plane
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// it's a bit too big for our scene, so scale it down
    model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
    objectShaderProgram->setMat4("model", model);
    objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
    DrawMesh("plane", normalMappingShaderProgramName);
}