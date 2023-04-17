#include "headers/BlendingTestScene.h"
#include <map>

void BlendingTestScene::SetupScene()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    blendingShaderProgramName = "blendingShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> objectSimpleShaders;

    objectSimpleShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    objectSimpleShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(blendingShaderProgramName, objectSimpleShaders);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    wallDiffuseMap = "wallDiffuseMap";
    grassTexture = "grassTexture";
    transparentWindowTexture = "transparentWindowTexture";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory);
    LoadTexture(wallDiffuseMap, "wall.jpg", textureDirectory);
    LoadTexture(grassTexture, "grass.png", textureDirectory);
    LoadTexture(transparentWindowTexture, "blending_transparent_window.png", textureDirectory);

    GetShaderProgram(blendingShaderProgramName)->setInt("material.diffuse", 0);

    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));

    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
    cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

    std::vector<glm::vec3> vegetationPositions;
    vegetationPositions.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    vegetationPositions.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    vegetationPositions.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetationPositions.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetationPositions.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["cubePositions"] = cubePositions;
    sceneAttributes["vegetationPositions"] = vegetationPositions;
}

void BlendingTestScene::RenderScene()
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
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
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    objectShaderProgram->setMat4("projection", projection);

    // FS stage Uniform inputs
    objectShaderProgram->setBool("texturing", true);

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
    DrawMesh("plane", blendingShaderProgramName);
    glBindVertexArray(0);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        DrawMesh("cube", blendingShaderProgramName);
    }
    {
        // Cube 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        DrawMesh("cube", blendingShaderProgramName);
    }

    // Draw Planes with grass textures
    // Set the material diffuse and specular maps
    // texture1 - Material diffuse
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("transparentWindowTexture"));
    
    std::map<float, glm::vec3> sortedList;
    for (unsigned int i = 0; i < sceneAttributes["vegetationPositions"].size(); i++)
    {
        float distance = glm::length(GetCamera("MainCamera")->Position - sceneAttributes["vegetationPositions"][i]);
        sortedList[distance] = sceneAttributes["vegetationPositions"][i];
    }

    unsigned int listSize = sortedList.size() - 1;
    for (std::map<float, glm::vec3>::reverse_iterator it = sortedList.rbegin(); it != sortedList.rend(); ++it)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, it->second); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        objectShaderProgram->setMat4("model", model);
        DrawMesh("plane", blendingShaderProgramName);
    }
    
    glBindVertexArray(0);
}