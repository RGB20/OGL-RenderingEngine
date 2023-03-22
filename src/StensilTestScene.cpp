#include "SensilTestScene.h"

void StensilTestScene::SetupScene() 
{
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    objectAndStensilShaderProgramName = "objectAndStensilShaderProgram";

    // Object shader program and other handlers
    std::string objectStensilVertexShaderPath = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    std::string objectStensilFragmentShaderPath = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(objectAndStensilShaderProgramName, objectStensilVertexShaderPath, objectStensilFragmentShaderPath);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    wallDiffuseMap = "wallDiffuseMap";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory);
    LoadTexture(wallDiffuseMap, "wall.jpg", textureDirectory);

    GetShaderProgram(objectAndStensilShaderProgramName)->setInt("material.diffuse", 0);

    // Add/Load Models
    AddPresetModels("cube", DEFAULT_MODELS::CUBE);
    AddPresetModels("plane", DEFAULT_MODELS::PLANE);

    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));

    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["cubePositions"] = cubePositions;
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
    UseShaderProgram(objectAndStensilShaderProgramName);
    std::shared_ptr<Shader> objectShaderProgram = GetShaderProgram(objectAndStensilShaderProgramName);

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
    objectShaderProgram->setBool("texturing", true);

    // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
    glStencilMask(0x00); // make sure we don't update the stencil buffer while drawing the floor

    // Planes VAO
    // Set the material diffuse and specular maps
    // texture1 - Material diffuse
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("wallDiffuseMap"));

    // Plane
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    objectShaderProgram->setMat4("model", model);
    DrawModel("plane", objectAndStensilShaderProgramName);
    glBindVertexArray(0);

    // 1st. render pass, draw objects as normal, writing to the stencil buffer
    // --------------------------------------------------------------------
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        DrawModel("cube", objectAndStensilShaderProgramName);
    }
    {
        // Cube 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        DrawModel("cube", objectAndStensilShaderProgramName);
    }

    // 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
    // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
    // the objects' size differences, making it look like borders.
    // -----------------------------------------------------------------------------------------------------------------------------
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    UseShaderProgram(objectAndStensilShaderProgramName); 
    {
        std::shared_ptr<Shader> stensilShaderProgram = GetShaderProgram(objectAndStensilShaderProgramName);
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
        stensilShaderProgram->setBool("texturing", false);
        stensilShaderProgram->setVec3("objectColor", glm::vec3(0.04, 0.28, 0.26));

        // Render the Cubes
        {
            // Cube 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// it's a bit too big for our scene, so scale it down
            stensilShaderProgram->setMat4("model", model);
            DrawModel("cube", objectAndStensilShaderProgramName);
        }
        {
            // Cube 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// it's a bit too big for our scene, so scale it down
            stensilShaderProgram->setMat4("model", model);
            DrawModel("cube", objectAndStensilShaderProgramName);
        }
        glBindVertexArray(0);
    }
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
}