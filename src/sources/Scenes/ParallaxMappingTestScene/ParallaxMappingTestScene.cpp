//#include "headers/Scenes/ParallaxMappingTestScene/ParallaxMappingTestScene.h"
//
//void ParallaxMappingTestScene::SetupScene()
//{
//    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));
//
//    shadowMappingShaderProgramName = "shadowMappingShaderProgram";
//    lightShaderProgramName = "lightShaderProgram";
//    depthMappingShaderProgramName = "depthMapShaderProgram";
//
//    // Object shader program and other handlers
//    std::unordered_map<SHADER_TYPES, std::string> shadowMappingShaders;
//
//    shadowMappingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\depthMapVertexShader.vs";
//    shadowMappingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\depthMapFragmentShader.fs";
//    AddShader(depthMappingShaderProgramName, shadowMappingShaders);
//
//    std::unordered_map<SHADER_TYPES, std::string> lightingShaders;
//
//    lightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
//    lightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
//    AddShader(lightShaderProgramName, lightingShaders);
//
//    std::unordered_map<SHADER_TYPES, std::string> objectLightingShaders;
//
//    objectLightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\shadowMapVertexShader.vs";
//    objectLightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\shadowMapFragmentShader.fs";
//    AddShader(shadowMappingShaderProgramName, objectLightingShaders);
//
//    // Load Textures
//    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\";
//    std::string containerSpecularTexMap = GetCurrentDir() + "\\textures\\";
//    std::string wallDiffuseTexMap = GetCurrentDir() + "\\textures\\";
//
//    containerDiffuseMap = "containerDiffuseMap";
//    containerSpecularMap = "containerSpecularMap";
//    wallDiffuseMap = "wallDiffuseMap";
//
//    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", containerDiffuseTexMap);
//    LoadTexture(containerSpecularMap, "containerSpecularMap.png", containerSpecularTexMap);
//    LoadTexture(wallDiffuseMap, "wall.jpg", wallDiffuseTexMap);
//
//    UseShaderProgram(shadowMappingShaderProgramName);
//    GetShaderProgram(shadowMappingShaderProgramName)->setInt("material.diffuse", 0);
//    GetShaderProgram(shadowMappingShaderProgramName)->setInt("material.specular", 1);
//    GetShaderProgram(shadowMappingShaderProgramName)->setInt("shadowMap", 2);
//
//    // Add/Load Models
//    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
//    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);
//
//    // Load Model parameters
//    std::vector<glm::vec3> planePositions;
//    planePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));
//
//    std::vector<glm::vec3> cubePositions;
//    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
//    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));
//
//    //std::vector<glm::vec3> pointLightPositions;
//    //pointLightPositions.push_back(glm::vec3(0.7f, 0.2f, 2.0f));
//    //pointLightPositions.push_back(glm::vec3(2.3f, -3.3f, -4.0f));
//    //pointLightPositions.push_back(glm::vec3(-4.0f, 2.0f, -12.0f));
//    //pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
//    //pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
//
//    // Directional Light direction
//    std::vector<glm::vec3> directionalLightPosition;
//    directionalLightPosition.push_back(glm::vec3(-2.0f, 4.0f, -1.0f));
//
//    std::vector<glm::vec3> lightColor;
//    lightColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
//
//    sceneAttributes["planePositions"] = planePositions;
//    sceneAttributes["cubePositions"] = cubePositions;
//    //sceneAttributes["pointLightPositions"] = pointLightPositions;
//    sceneAttributes["directionalLightPosition"] = directionalLightPosition;
//    sceneAttributes["lightColor"] = lightColor;
//
//    // Setup the Depth Map FBP
//    glGenFramebuffers(1, &depthMapFBO);
//
//    glGenTextures(1, &depthMap);
//    glBindTexture(GL_TEXTURE_2D, depthMap);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
//    glDrawBuffer(GL_NONE);
//    glReadBuffer(GL_NONE);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
//
//void ParallaxMappingTestScene::RenderScene(unsigned int deferredQuadFrameBuffer)
//{
//    // Render the scene to generate ShadowMap
//    {
//        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
//        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
//        glClear(GL_DEPTH_BUFFER_BIT);
//        //glCullFace(GL_FRONT);
//
//        glm::mat4 lightView = glm::mat4(1.0f);
//        glm::mat4 lightProjection = glm::mat4(1.0f);
//
//        // Render Objects
//        UseShaderProgram(depthMappingShaderProgramName);
//        std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(depthMappingShaderProgramName);
//
//        // Light Space View Matrix
//        lightView = glm::lookAt(sceneAttributes["directionalLightPosition"][0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
//        objectShaderProgram->setMat4("view", lightView);
//        // Light Space Projection Matrix
//        float near_plane = 1.0f, far_plane = 7.5f;
//        //float near_plane = 0.1f, far_plane = 100.0f;
//        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
//        //projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
//        objectShaderProgram->setMat4("projection", lightProjection);
//
//        // Plane
//        glm::mat4 model = glm::mat4(1.0f);
//        model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
//        model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
//        objectShaderProgram->setMat4("model", model);
//        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//        DrawMesh("plane", depthMappingShaderProgramName);
//
//        // Render the Cubes
//        {
//            // Cube 1
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
//            objectShaderProgram->setMat4("model", model);
//            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//            DrawMesh("cube", depthMappingShaderProgramName);
//        }
//        {
//            // Cube 2
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
//            objectShaderProgram->setMat4("model", model);
//            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//            DrawMesh("cube", depthMappingShaderProgramName);
//        }
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    }
//
//    // Render the scene as normal
//    {
//        glm::mat4 view = glm::mat4(1.0f);
//        glm::mat4 projection = glm::mat4(1.0f);
//        // RENDERING COMMANDS
//
//        // clear render targets
//        // ------
//        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//        glBindFramebuffer(GL_FRAMEBUFFER, deferredQuadFrameBuffer);
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        //glCullFace(GL_BACK);
//
//        // Render Objects
//        UseShaderProgram(shadowMappingShaderProgramName);
//        std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(shadowMappingShaderProgramName);
//
//        // VS stage Uniform inputs
//        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
//        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
//        // You can set the uniforms once or every frame
//        // View
//        objectShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
//        // Projection
//        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
//        objectShaderProgram->setMat4("projection", projection);
//
//        // lightSpaceMatrix
//        // Light Space Projection Matrix
//        float near_plane = 1.0f, far_plane = 7.5f;
//        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
//        // Light Space View Matrix
//        glm::mat4 lightView = glm::lookAt(sceneAttributes["directionalLightPosition"][0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
//        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
//        objectShaderProgram->setMat4("lightSpaceMatrix", lightSpaceMatrix);
//
//        // FS stage Uniform inputs
//        objectShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);
//
//        objectShaderProgram->setFloat("material.shininess", 32.0f);
//
//        //DIRECTIONAL LIGHT
//        objectShaderProgram->setVec3("lightPos", sceneAttributes["directionalLightPosition"][0]);
//
//        // Planes VAO
//        // Set the material diffuse and specular maps
//        // texture1 - Material diffuse
//        // texture2 - Material specular 
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, GetTextureID("wallDiffuseMap"));
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, depthMap);
//
//        // Plane
//        glm::mat4 model = glm::mat4(1.0f);
//        model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
//        model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
//        objectShaderProgram->setMat4("model", model);
//        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//        DrawMesh("plane", shadowMappingShaderProgramName);
//
//        // Render the Cubes
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, depthMap);
//        {
//            // Cube 1
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
//            objectShaderProgram->setMat4("model", model);
//            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//            DrawMesh("cube", shadowMappingShaderProgramName);
//        }
//        {
//            // Cube 2
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
//            objectShaderProgram->setMat4("model", model);
//            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
//            DrawMesh("cube", shadowMappingShaderProgramName);
//        }
//    }
//}