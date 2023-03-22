#include "DepthTestingScene.h"

void DepthTestingScene::SetupScene()
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    depthTestShaderProgramName = "depthTestShaderProgram";

    // Object shader program and other handlers
    std::string depthTestShaderVertexPath = GetCurrentDir() + "\\shaders\\depthTestingVertexShader.vs";
    std::string depthTestFragmentShaderPath = GetCurrentDir() + "\\shaders\\depthTestingFragmentShader.fs";
    AddShader(depthTestShaderProgramName, depthTestShaderVertexPath, depthTestFragmentShaderPath);

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

void DepthTestingScene::RenderScene()
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render Objects
    UseShaderProgram(depthTestShaderProgramName);
    std::shared_ptr<Shader> depthTestShaderProgram = GetShaderProgram(depthTestShaderProgramName);

    // VS stage Uniform inputs
    // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    // You can set the uniforms once or every frame
    // View
    depthTestShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    depthTestShaderProgram->setMat4("projection", projection);

    // Draw Plane
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sceneAttributes["planePositions"][0]); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
    depthTestShaderProgram->setMat4("model", model);
    DrawModel("plane", depthTestShaderProgramName);
    glBindVertexArray(0);

    // Render the Cubes
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        depthTestShaderProgram->setMat4("model", model);
        DrawModel("cube", depthTestShaderProgramName);
    }
    {
        // Cube 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        depthTestShaderProgram->setMat4("model", model);
        DrawModel("cube", depthTestShaderProgramName);
    }
}