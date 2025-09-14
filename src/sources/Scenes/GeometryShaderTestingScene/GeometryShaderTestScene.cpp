#include "headers/Scenes/GeometryShaderTestingScene/GeometryShaderTestScene.h"

void GeometryShaderTestScene::SetupScene()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    objectShaderProgramName = "objectShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> objectShaders;

    objectShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    objectShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(objectShaderProgramName, objectShaders);

    // The vertex normals are visualized using geometry shader
    vertexNormalsShaderProgramName = "geometryShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> geometryShaderTestshaders;

    geometryShaderTestshaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\visualizeNormalsVertexShader.vs";
    geometryShaderTestshaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\visualizeNormalsFragmentShader.fs";
    geometryShaderTestshaders[SHADER_TYPES::GEOMETRY_SHADER] = GetCurrentDir() + "\\shaders\\visualizeNormalsGeometryShader.gs";
    AddShader(vertexNormalsShaderProgramName, geometryShaderTestshaders);

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

    std::string skyboxtextureDirectory = GetCurrentDir() + "\\textures\\skyboxTextures\\OceanAndSky\\";

    std::vector<std::string> cubemapFaces;
    cubemapFaces.push_back("right.jpg");
    cubemapFaces.push_back("left.jpg");
    cubemapFaces.push_back("top.jpg");
    cubemapFaces.push_back("bottom.jpg");
    cubemapFaces.push_back("front.jpg");
    cubemapFaces.push_back("back.jpg");
    LoadCubeMapTexture("skyboxCubeMap", cubemapFaces, skyboxtextureDirectory);

    GetShaderProgram(skyboxShaderProgramName)->setInt("skyboxTexture", 0);

    // Add/Load Models
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);

    // Load Model parameters
    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    sceneAttributes["cubePositions"] = cubePositions;
}

void GeometryShaderTestScene::RenderScene()
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
    objectShaderProgram->setBool("texturing", true);

    // Render the Cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureID("containerDiffuseMap"));
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        DrawMesh("cube", objectShaderProgramName);
    }

    // Draw per vertex normals using Geometry Shader
    UseShaderProgram(vertexNormalsShaderProgramName);
    std::shared_ptr<Shader> vertexNormalsShaderProgram = GetShaderProgram(vertexNormalsShaderProgramName);

    // VS stage Uniform inputs
    // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
    // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
    // You can set the uniforms once or every frame
    // View
    vertexNormalsShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
    // Projection
    projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
    vertexNormalsShaderProgram->setMat4("projection", projection);

    // Render the Cube Normals
    {
        // Cube 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneAttributes["cubePositions"][0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        vertexNormalsShaderProgram->setMat4("model", model);
        DrawMesh("cube", vertexNormalsShaderProgramName);
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
    DrawMesh("cube", skyboxShaderProgramName);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}