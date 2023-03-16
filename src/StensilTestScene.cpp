#include "SensilTestScene.h"

glm::vec3 planePositions[] = {
    glm::vec3(0.0f,  -0.5f,  0.0f)
};

glm::vec3 cubePositions[] = {
    glm::vec3(-1.0f,  0.0f,  1.0f),
    glm::vec3(2.0f, 0.0f, 0.0f)
};

glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

// Directional Light direction
glm::vec3 directionalLightDirection(-0.2f, -1.0f, -0.3f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

void StensilTestScene::SetupScene() 
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    objectShaderProgramName = "objectBaseShaderProgram";
    stensilShaderProgramName = "stensilShaderProgram";
    lightShaderProgramName = "lightShaderProgram";

    // Object shader program and other handlers
    std::string objectStensilVertexShaderPath = GetCurrentDir() + "\\shaders\\objectStensilShader.vs";
    std::string objectStensilFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectStensilShader.fs";
    AddShader(stensilShaderProgramName, objectStensilVertexShaderPath, objectStensilFragmentShaderPath);

    std::string lightVertexShaderPath = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    std::string lightFragmentShaderPath = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    AddShader(lightShaderProgramName, lightVertexShaderPath, lightFragmentShaderPath);

    std::string objectVertexShaderPath = GetCurrentDir() + "\\shaders\\objectVertexShader.vs";
    std::string objectFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectFragmentShader.fs";
    AddShader(objectShaderProgramName, objectVertexShaderPath, objectFragmentShaderPath);

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
    GetShaderProgram(stensilShaderProgramName)->setInt("material.diffuse", 0);

    // Add/Load Models
    AddPresetModels("cube", DEFAULT_MODELS::CUBE);
    AddPresetModels("plane", DEFAULT_MODELS::PLANE);
}

void StensilTestScene::RenderScene()
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render Objects
    UseShaderProgram(objectShaderProgramName);// objectShaderProgram->use();
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
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
    objectShaderProgram->setVec3("dirLight.direction", directionalLightDirection);
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
        objectShaderProgram->setVec3("pointLights[" + number_str + "].position", pointLightPositions[i]);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].constant", 1.0f);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].linear", 0.09f);
        objectShaderProgram->setFloat("pointLights[" + number_str + "].quadratic", 0.032f);
        objectShaderProgram->setVec3("pointLights[" + number_str + "].ambient", ambientColor);
        objectShaderProgram->setVec3("pointLights[" + number_str + "].diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram->setVec3("pointLights[" + number_str + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
    glStencilMask(0x00); // make sure we don't update the stencil buffer while drawing the floor

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
    model = glm::translate(model, planePositions[0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    objectShaderProgram->setMat4("model", model);
    objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
    DrawModel("plane", objectShaderProgramName);
    glBindVertexArray(0);

    // 1st. render pass, draw objects as normal, writing to the stencil buffer
    // --------------------------------------------------------------------
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerSpecularMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        DrawModel("cube", objectShaderProgramName);
    }
    {
        // Cube 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        DrawModel("cube", objectShaderProgramName);
    }

    // 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
    // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
    // the objects' size differences, making it look like borders.
    // -----------------------------------------------------------------------------------------------------------------------------
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    UseShaderProgram(stensilShaderProgramName); //objectStensilShaderProgram.use(); 
    {
        std::shared_ptr<Shader> stensilShaderProgram = GetShaderProgram(stensilShaderProgramName);
        // VS stage Uniform inputs
        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
        // You can set the uniforms once or every frame
        // View
        stensilShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        stensilShaderProgram->setMat4("projection", projection);

        // FS stage Uniform inputs
        stensilShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

        // Set the material diffuse and specular maps
        // texture1 - Material diffuse
        // texture2 - Material specular 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);
        stensilShaderProgram->setVec3("material.specular", glm::vec3(0.5, 0.5, 0.5));
        stensilShaderProgram->setFloat("material.shininess", 32.0f);

        //DIRECTIONAL LIGHT
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
        stensilShaderProgram->setVec3("dirLight.direction", directionalLightDirection);
        stensilShaderProgram->setVec3("dirLight.ambient", ambientColor);
        stensilShaderProgram->setVec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
        stensilShaderProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // SPOT LIGHT
        stensilShaderProgram->setVec3("spotLight.position", GetCamera("MainCamera")->Position);
        stensilShaderProgram->setVec3("spotLight.direction", GetCamera("MainCamera")->Front);
        stensilShaderProgram->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        stensilShaderProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

        stensilShaderProgram->setVec3("spotLight.ambient", ambientColor);
        stensilShaderProgram->setVec3("spotLight.diffuse", diffuseColor); // darken diffuse light a bit
        stensilShaderProgram->setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        stensilShaderProgram->setFloat("spotLight.constant", 1.0f);
        stensilShaderProgram->setFloat("spotLight.linear", 0.09f);
        stensilShaderProgram->setFloat("spotLight.quadratic", 0.032f);

        // POINT LIGHT
        // This should be the same as the number of points lights we statically define in the shader
        // TODO: Make this dynamic
        int pointLights = 4;
        for (int i = 0; i < pointLights; ++i)
        {
            std::string number_str = std::to_string(i);
            stensilShaderProgram->setVec3("pointLights[" + number_str + "].position", pointLightPositions[i]);
            stensilShaderProgram->setFloat("pointLights[" + number_str + "].constant", 1.0f);
            stensilShaderProgram->setFloat("pointLights[" + number_str + "].linear", 0.09f);
            stensilShaderProgram->setFloat("pointLights[" + number_str + "].quadratic", 0.032f);
            stensilShaderProgram->setVec3("pointLights[" + number_str + "].ambient", ambientColor);
            stensilShaderProgram->setVec3("pointLights[" + number_str + "].diffuse", diffuseColor); // darken diffuse light a bit
            stensilShaderProgram->setVec3("pointLights[" + number_str + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // Render the Cubes
        {
            // Cube 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[0]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// it's a bit too big for our scene, so scale it down
            stensilShaderProgram->setMat4("model", model);
            stensilShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawModel("cube", stensilShaderProgramName);
        }
        {
            // Cube 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// it's a bit too big for our scene, so scale it down
            stensilShaderProgram->setMat4("model", model);
            stensilShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawModel("cube", stensilShaderProgramName);
        }
        glBindVertexArray(0);
    }
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);


    // RENDER LIGHTS
    UseShaderProgram(lightShaderProgramName);
    std::shared_ptr<Shader> lightShaderProgram = GetShaderProgram(lightShaderProgramName);
    // Set the transform uniforms once or every frame

    lightShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    lightShaderProgram->setMat4("projection", projection);

    // Set light color uniform
    lightShaderProgram->setVec3("lightColor", lightColor);

    //glBindVertexArray(lightVAO);
    for (int i = 0; i < pointLights; ++i)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, pointLightPositions[i]);
        lightModel = glm::scale(lightModel, glm::vec3(0.1f));
        lightShaderProgram->setMat4("model", lightModel);

        DrawModel("cube", lightShaderProgramName);
    }
}