#include "headers/Scenes/ShadowMappingMegaScene/ShadowMappingMegaScene.h"
#include <GLFW/glfw3.h>

void ShadowMappingMegaScene::SetupScene()
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(-10.0f, 5.0f, 10.0f)));

    shadowMappingShaderProgramName = "shadowMappingShaderProgram";
    depthMappingShaderProgramName = "depthMapShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> depthMappingShaders;

    depthMappingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\depthMapMegaVertexShader.vs";
    depthMappingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\depthMapMegaFragmentShader.fs";
    AddShader(depthMappingShaderProgramName, depthMappingShaders);

    std::unordered_map<SHADER_TYPES, std::string> objectLightingShaders;

    objectLightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\shadowMapMegaVertexShader.vs";
    objectLightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\shadowMapMegaFragmentShader.fs";
    AddShader(shadowMappingShaderProgramName, objectLightingShaders);

    // Load Textures
    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\";
    std::string containerSpecularTexMap = GetCurrentDir() + "\\textures\\";
    std::string planeTextureMap = GetCurrentDir() + "\\textures\\Ground068_1K-JPG\\";

    containerDiffuseMap = "containerDiffuseMap";
    containerSpecularMap = "containerSpecularMap";
    planeTexture = "planeTexture";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", containerDiffuseTexMap);
    LoadTexture(containerSpecularMap, "containerSpecularMap.png", containerSpecularTexMap);
    LoadTexture(planeTexture, "color.jpg", planeTextureMap);

    UseShaderProgram(shadowMappingShaderProgramName);
    GetShaderProgram(shadowMappingShaderProgramName)->setInt("material.diffuse", 0);
    GetShaderProgram(shadowMappingShaderProgramName)->setInt("material.specular", 1);
    GetShaderProgram(shadowMappingShaderProgramName)->setInt("shadowMap", 2);

    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Load Model parameters
    float planeScaleValue = 1.0f;
    std::vector<glm::vec3> planePositions;
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 20; y++) {
            float xPos = 0.0f + (x-10) * planeScaleValue * 2;
            float yPos = 0.0f + (y-10) * planeScaleValue * 2;
            planePositions.push_back(glm::vec3(xPos, 10.0f, yPos));
        }
    }

    std::vector<glm::vec3> planeScale;
    planeScale.push_back(glm::vec3(planeScaleValue));

    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

    //std::vector<glm::vec3> pointLightPositions;
    //pointLightPositions.push_back(glm::vec3(0.7f, 0.2f, 2.0f));
    //pointLightPositions.push_back(glm::vec3(2.3f, -3.3f, -4.0f));
    //pointLightPositions.push_back(glm::vec3(-4.0f, 2.0f, -12.0f));
    //pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
    //pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));

    // Directional Light direction
    std::vector<glm::vec3> directionalLightPosition;
    directionalLightPosition.push_back(glm::vec3(-2.0f, 4.0f, -1.0f));

    std::vector<glm::vec3> lightColor;
    lightColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["planeScale"] = planeScale;
    sceneAttributes["cubePositions"] = cubePositions;
    //sceneAttributes["pointLightPositions"] = pointLightPositions;
    sceneAttributes["directionalLightPosition"] = directionalLightPosition;
    sceneAttributes["lightColor"] = lightColor;

    // Setup the Depth Map FBO
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set time to an initial value
    time = 0;
}

void ShadowMappingMegaScene::DemoKeyPressed(uint16_t keyCode) {

    if (keyCode == GLFW_KEY_T) dropTerrain = true;
    if (keyCode == GLFW_KEY_Y) dropBoxes = true;
    if (keyCode == GLFW_KEY_U) addSkyBox = true;
}

void ShadowMappingMegaScene::RenderScene(unsigned int deferredQuadFrameBuffer)
{
    // Render the scene to generate ShadowMap
    {
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //glCullFace(GL_BACK);

        glm::mat4 lightView = glm::mat4(1.0f);
        glm::mat4 lightProjection = glm::mat4(1.0f);

        // Render Objects
        UseShaderProgram(depthMappingShaderProgramName);
        std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(depthMappingShaderProgramName);

        // Light Space View Matrix
        lightView = glm::lookAt(sceneAttributes["directionalLightPosition"][0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        objectShaderProgram->setMat4("view", lightView);
        // Light Space Projection Matrix
        float near_plane = 1.0f, far_plane = 7.5f;
        //float near_plane = 0.1f, far_plane = 100.0f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        //projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        objectShaderProgram->setMat4("projection", lightProjection);

        // Plane
        for (int i = 0; i < sceneAttributes["planePositions"].size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["planePositions"][i]); // translate it down so it's at the center of the scene
            model = glm::scale(model, sceneAttributes["planeScale"][0]);	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawMesh("plane", depthMappingShaderProgramName);
        }
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
        //model = glm::scale(model, sceneAttributes["planeScale"][0]);	// it's a bit too big for our scene, so scale it down
        //objectShaderProgram->setMat4("model", model);
        //objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        //DrawMesh("plane", depthMappingShaderProgramName);

        //glCullFace(GL_FRONT);

        // Render the Cubes
        {
            // Cube 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawMesh("cube", depthMappingShaderProgramName);
        }
        {
            // Cube 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawMesh("cube", depthMappingShaderProgramName);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Render the scene as normal
    {
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
        UseShaderProgram(shadowMappingShaderProgramName);
        std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(shadowMappingShaderProgramName);

        // VS stage Uniform inputs
        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
        // You can set the uniforms once or every frame
        // View
        objectShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        objectShaderProgram->setMat4("projection", projection);

        // lightSpaceMatrix
        // Light Space Projection Matrix
        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        // Light Space View Matrix
        glm::mat4 lightView = glm::lookAt(sceneAttributes["directionalLightPosition"][0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        objectShaderProgram->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // FS stage Uniform inputs
        objectShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

        objectShaderProgram->setFloat("material.shininess", 32.0f);

        //DIRECTIONAL LIGHT
        objectShaderProgram->setVec3("lightPos", sceneAttributes["directionalLightPosition"][0]);

        // Plane
        if (dropTerrain) {
            // Planes VAO
            // Set the material diffuse and specular maps
            // texture1 - Material diffuse
            // texture2 - Material specular 
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, GetTextureID("planeTexture"));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, depthMap);

            time += 0.01f;
            for (int i = 0; i < sceneAttributes["planePositions"].size(); i++) {
                glm::mat4 model = glm::mat4(1.0f);
                float height = sceneAttributes["planePositions"][i].y;
                if (height > -0.5) {
                    height -= time / i;
                    sceneAttributes["planePositions"][i].y = height;
                }
                else {
                    sceneAttributes["planePositions"][i].y = -0.5f;
                }
                model = glm::translate(model, sceneAttributes["planePositions"][i]); // translate it down so it's at the center of the scene
                model = glm::scale(model, sceneAttributes["planeScale"][0]);	// it's a bit too big for our scene, so scale it down
                objectShaderProgram->setMat4("model", model);
                objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
                objectShaderProgram->setFloat("UVScale", 1.0f);
                DrawMesh("plane", shadowMappingShaderProgramName);
            }
        }

        if (dropBoxes) {
            // Render the Cubes
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            {
                // Cube 1
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
                model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
                objectShaderProgram->setMat4("model", model);
                objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
                objectShaderProgram->setFloat("UVScale", 1.0f);
                DrawMesh("cube", shadowMappingShaderProgramName);
            }
            {
                // Cube 2
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
                model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
                objectShaderProgram->setMat4("model", model);
                objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
                objectShaderProgram->setFloat("UVScale", 1.0f);
                DrawMesh("cube", shadowMappingShaderProgramName);
            }
        }
    }
}