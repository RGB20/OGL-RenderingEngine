#include "headers/InstancingTestScene.h"

void InstancingTestScene::SetupScene()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    // The vertex normals are visualized using geometry shader
    instancingShaderProgramName = "instancingShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> instancingTestshaders;

    instancingTestshaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\instancingVertexShader.vs";
    instancingTestshaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(instancingShaderProgramName, instancingTestshaders);

    skyboxShaderProgramName = "skyboxShaderProgram";

    std::unordered_map<SHADER_TYPES, std::string> skyboxShaders;

    skyboxShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\skyboxVertexShader.vs";
    skyboxShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\skyboxFragmentShader.fs";
    AddShader(skyboxShaderProgramName, skyboxShaders);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    containerDiffuseMap = "containerDiffuseMap";
    skyboxTexture = "skyboxTexture";

    LoadTexture(containerDiffuseMap, "containerDiffuseMap.png", textureDirectory);

    std::string skyboxtextureDirectory = GetCurrentDir() + "\\textures\\skyboxTextures\\Galaxy\\";

    std::vector<std::string> cubemapFaces;
    cubemapFaces.push_back("right.png");
    cubemapFaces.push_back("left.png");
    cubemapFaces.push_back("top.png");
    cubemapFaces.push_back("bottom.png");
    cubemapFaces.push_back("front.png");
    cubemapFaces.push_back("back.png");
    LoadCubeMapTexture("skyboxCubeMap", cubemapFaces, skyboxtextureDirectory);

    GetShaderProgram(skyboxShaderProgramName)->setInt("skyboxTexture", 0);

    // Add/Load Models
    AddPresetModels("cube", DEFAULT_MODELS::CUBE);

    // Load Model parameters
    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    sceneAttributes["cubePositions"] = cubePositions;
}

void InstancingTestScene::RenderScene()
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // RENDERING COMMANDS

    // clear render targets
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render Objects
    // Draw the base CUBE object
    UseShaderProgram(instancingShaderProgramName);
    std::shared_ptr<Shader> instancingShaderProgram = GetShaderProgram(instancingShaderProgramName);

    // VS stage Uniform inputs
    // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    // You can set the uniforms once or every frame
    // View
    instancingShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    instancingShaderProgram->setMat4("projection", projection);

    // FS stage Uniform inputs
    instancingShaderProgram->setBool("texturing", true);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        instancingShaderProgram->setMat4("model", model);
        DrawModel("cube", instancingShaderProgramName);
    }

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
    DrawModel("cube", skyboxShaderProgramName);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}