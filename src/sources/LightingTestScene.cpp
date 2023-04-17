#include "headers/LightingTestScene.h"

void LightingTestScene::SetupScene()
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    objectShaderProgramName = "objectBaseShaderProgram";
    lightShaderProgramName = "lightShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> lightingShaders;

    lightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    lightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    AddShader(lightShaderProgramName, lightingShaders);

    std::unordered_map<SHADER_TYPES, std::string> objectLightingShaders;

    objectLightingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\objectLightingVertexShader.vs";
    objectLightingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\objectLightingFragmentShader.fs";
    AddShader(objectShaderProgramName, objectLightingShaders);

    // Load Textures
    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\";
    std::string containerSpecularTexMap = GetCurrentDir() + "\\textures\\";
    std::string wallDiffuseTexMap = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    containerSpecularMap = "containerSpecularMap";
    wallDiffuseMap = "wallDiffuseMap";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", containerDiffuseTexMap);
    LoadTexture(containerSpecularMap, "containerSpecularMap.png", containerSpecularTexMap);
    LoadTexture(wallDiffuseMap, "wall.jpg", wallDiffuseTexMap);

    GetShaderProgram(objectShaderProgramName)->setInt("material.diffuse", 0);
    GetShaderProgram(objectShaderProgramName)->setInt("material.specular", 1);

    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);
    
    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(0.0f,  -0.5f,  0.0f));

    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

    std::vector<glm::vec3> pointLightPositions;
    pointLightPositions.push_back(glm::vec3(0.7f, 0.2f, 2.0f));
    pointLightPositions.push_back(glm::vec3(2.3f, -3.3f, -4.0f));
    pointLightPositions.push_back(glm::vec3(-4.0f, 2.0f, -12.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));

    // Directional Light direction
    std::vector<glm::vec3> directionalLightDirection;
    directionalLightDirection.push_back(glm::vec3(-0.2f, -1.0f, -0.3f));
    
    std::vector<glm::vec3> lightColor;
    lightColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["cubePositions"] = cubePositions;
    sceneAttributes["pointLightPositions"] = pointLightPositions;
    sceneAttributes["directionalLightDirection"] = directionalLightDirection;
    sceneAttributes["lightColor"] = lightColor;
}

void LightingTestScene::RenderScene()
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Objects
    UseShaderProgram(objectShaderProgramName);
    std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(objectShaderProgramName);

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
    glm::vec3 diffuseColor = sceneAttributes["lightColor"][0] * glm::vec3(0.5f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
    objectShaderProgram->setVec3("dirLight.direction", sceneAttributes["directionalLightDirection"][0]);
    objectShaderProgram->setVec3("dirLight.ambient", ambientColor);
    objectShaderProgram->setVec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
    objectShaderProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    // SPOT LIGHT
    objectShaderProgram->setVec3("spotLight.position", GetCamera("MainCamera")->Position);
    objectShaderProgram->setVec3("spotLight.direction", GetCamera("MainCamera")->Front);
    objectShaderProgram->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    objectShaderProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

    objectShaderProgram->setVec3("spotLight.ambient", ambientColor);
    objectShaderProgram->setVec3("spotLight.diffuse", diffuseColor); // darken diffuse light a bit
    objectShaderProgram->setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    objectShaderProgram->setFloat("spotLight.constant", 1.0f);
    objectShaderProgram->setFloat("spotLight.linear", 0.09f);
    objectShaderProgram->setFloat("spotLight.quadratic", 0.032f);

    // POINT LIGHT
    // This should be the same as the number of points lights we statically define in the shader
    // TODO: Make this dynamic
    int pointLights = 4;
    for (int i = 0; i < pointLights; ++i)
    {
        std::string number_str = std::to_string(i);
        objectShaderProgram->setVec3("pointLights[" + number_str + "].position",  sceneAttributes["pointLightPositions"][i]);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].constant", 1.0f);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].linear", 0.09f);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].quadratic", 0.032f);
        objectShaderProgram->setVec3("pointLights[" + number_str + "].ambient", ambientColor);
        objectShaderProgram->setVec3("pointLights[" + number_str + "].diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram->setVec3("pointLights[" + number_str + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // Planes VAO
    // Set the material diffuse and specular maps
    // texture1 - Material diffuse
    // texture2 - Material specular 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("wallDiffuseMap"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));

    // Plane
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    objectShaderProgram->setMat4("model", model);
    objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
    DrawMesh("plane", objectShaderProgramName);
    glBindVertexArray(0);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        DrawMesh("cube", objectShaderProgramName);
    }
    {
        // Cube 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        DrawMesh("cube", objectShaderProgramName);
    }

    // RENDER LIGHTS
    UseShaderProgram(lightShaderProgramName);
    std::shared_ptr<Shader> lightShaderProgram = GetShaderProgram(lightShaderProgramName);
    // Set the transform uniforms once or every frame

    lightShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    lightShaderProgram->setMat4("projection", projection);

    // Set light color uniform
    lightShaderProgram->setVec3("lightColor", sceneAttributes["lightColor"][0]);

    //glBindVertexArray(lightVAO);
    for (int i = 0; i < pointLights; ++i)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, sceneAttributes["pointLightPositions"][i]);
        lightModel = glm::scale(lightModel, glm::vec3(0.1f));
        lightShaderProgram->setMat4("model", lightModel);

        DrawMesh("cube", lightShaderProgramName);
    }
}