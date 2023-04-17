#include "headers/InstancingTestScene.h"
#include <GLFW/glfw3.h>

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

    objectShaderProgramName = "objectShaderProgram";

    std::unordered_map<SHADER_TYPES, std::string> objectShaders;

    objectShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\simpleVertexShader.vs";
    objectShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\simpleFragmentShader.fs";
    AddShader(objectShaderProgramName, objectShaders);

    // Load Textures
    std::string textureDirectory = GetCurrentDir() + "\\textures\\";

    std::string skyboxtextureDirectory = GetCurrentDir() + "\\textures\\skyboxTextures\\Galaxy\\";

    std::vector<std::string> cubemapFaces;
    cubemapFaces.push_back("right.png");
    cubemapFaces.push_back("left.png");
    cubemapFaces.push_back("top.png");
    cubemapFaces.push_back("bottom.png");
    cubemapFaces.push_back("front.png");
    cubemapFaces.push_back("back.png");
    skyboxTextureName = "skyboxCubeMapTexture";
    LoadCubeMapTexture(skyboxTextureName, cubemapFaces, skyboxtextureDirectory);

    GetShaderProgram(skyboxShaderProgramName)->setInt("skyboxTexture", 0);

    // Add/Load Models
    std::string asteroidModelPath = "C:\\Users\\rudra\\Documents\\Projects\\OGL-RenderingEngine\\Models\\rock\\rock.obj";
    std::string planetModelPath = "C:\\Users\\rudra\\Documents\\Projects\\OGL-RenderingEngine\\Models\\planet\\planet.obj";
    AddMesh("planet", planetModelPath);
    AddMesh("asteroid", asteroidModelPath);
    AddPresetMesh("cube", DEFAULT_MESHES::CUBE);

    // Load Model parameters
    std::vector<glm::vec3> planetPositions;
    planetPositions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    planetPositions.push_back(glm::vec3(50.0f, 0.0f, 50.0f));

    sceneAttributes["planetPositions"] = planetPositions;

    // For the asteroid field around the planet we need to create 1 model matrix for each asteroid
    asteroidCount = 10000;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[asteroidCount];
    srand(glfwGetTime()); // initialize random seed	
    float radius = 20;
    float offset = 5.0f;
    unsigned int asteroidIndex = 0;
    for (unsigned int planetIndex = 0; planetIndex < planetPositions.size(); planetIndex++)
    {
        for (unsigned int i = 0; i < ( asteroidCount / planetPositions.size() ) ; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)asteroidCount * 360.0f;
            float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;
            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;
            model = glm::translate(model, planetPositions[planetIndex] + glm::vec3(x, y, z));

            // 2. scale: scale between 0.05 and 0.25f
            float scale = (rand() % 3) / 100.0f + 0.05;
            model = glm::scale(model, glm::vec3(scale));

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            float rotAngle = (rand() % 360);
            model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

            // 4. now add to list of matrices
            modelMatrices[asteroidIndex] = model;

            asteroidIndex++;
        }
    }
    

    // Create the instance buffer
    unsigned int instanceBuffer;
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, asteroidCount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    unsigned int asteroidVAO = GetMesh("asteroid")->GetMeshVAO();
    glBindVertexArray(asteroidVAO);
    // vertex attributes
    std::size_t vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
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
    // Draw the asteroids
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
    {
        // Asteroids
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, sceneAttributes["planetPositions"][0]); // translate it down so it's at the center of the scene
        //model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        //instancingShaderProgram->setMat4("model", model);
        //glBindVertexArray(GetMesh("asteroid")->GetMeshVAO());
        //glDrawElementsInstanced(GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
        DrawMesh("asteroid", instancingShaderProgramName, true, asteroidCount);
    }

    // Draw the planet
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
    {
        // Planets
        for (unsigned int planetIndex = 0; planetIndex < sceneAttributes["planetPositions"].size(); planetIndex++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneAttributes["planetPositions"][planetIndex]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            DrawMesh("planet", objectShaderProgramName);
        }
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
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetTextureID(skyboxTextureName));
    DrawMesh("cube", skyboxShaderProgramName);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}