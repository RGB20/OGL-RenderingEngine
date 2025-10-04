#include "headers/Scenes/HDRMappingTestScene/HDRMappingTestScene.h"
#include <tuple>

void HDRMappingTestScene::SetupScene()
{
    AddCamera("MainCamera", std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 5.0f)));

    HDRMappingShaderProgramName = "HDRMappingShaderProgram";

    // Object shader program and other handlers
    std::unordered_map<SHADER_TYPES, std::string> parallaxMappingShaders;

    parallaxMappingShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\HDRMappingLightingVertexShader.vs";
    parallaxMappingShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\HDRMappingLightingFragmentShader.fs";
    AddShader(HDRMappingShaderProgramName, parallaxMappingShaders);

    // Load Textures
    std::string WallDisffuseMapTexPath = GetCurrentDir() + "\\textures\\WoodFloor051_1K-JPG\\";
    //std::string WallNormalMapTexPath = GetCurrentDir() + "\\textures\\WoodFloor051_1K-JPG\\";
    //std::string WallDisplacementMapPath = GetCurrentDir() + "\\textures\\WoodFloor051_1K-JPG\\";

    WallDiffuseMap = "wallDiffuseMap";
    //WallNormalMap = "wallNormalMap";
    //WallDisplacementMap = "wallDisplacementMap";

    LoadTexture(WallDiffuseMap, "WoodFloor051_1K-JPG_Color.jpg", WallDisffuseMapTexPath, true);
    //LoadTexture(WallNormalMap, "WoodFloor051_1K-JPG_NormalGL.jpg", WallNormalMapTexPath);
    //LoadTexture(WallDisplacementMap, "WoodFloor051_1K-JPG_Displacement.jpg", WallDisplacementMapPath);

    UseShaderProgram(HDRMappingShaderProgramName);
    GetShaderProgram(HDRMappingShaderProgramName)->setInt("material.diffuseMap", 0);
    //GetShaderProgram(HDRMappingShaderProgramName)->setInt("material.normalMap", 1);
    //GetShaderProgram(HDRMappingShaderProgramName)->setInt("material.depthMap", 2);

    // Add/Load Models
    AddPresetMesh("plane", DEFAULT_MESHES::PLANE);

    // Load Model parameters
    std::vector<glm::vec3> planePositions;
    planePositions.push_back(glm::vec3(-2.0f, 0.0f, -2.0f)); // Left wall
    planePositions.push_back(glm::vec3(2.0f, 0.0f, -2.0f));  // Right wall
    planePositions.push_back(glm::vec3(0.0f, 2.0f, -2.0f));  // Up wall
    planePositions.push_back(glm::vec3(0.0f, -2.0f, -2.0f));  // Bottom wall
    planePositions.push_back(glm::vec3(0.0f, 0.0f, -16.0f));  // Back wall

    std::vector<glm::vec3> planeRotations;
    planeRotations.push_back(glm::vec3(0.0f, 0.0f, -90.0f)); // Left wall rotation
    planeRotations.push_back(glm::vec3(0.0f, 0.0f, 90.0f)); // Right wall rotation
    planeRotations.push_back(glm::vec3(180.0f, 0.0f, 0.0f)); // Up wall rotation
    planeRotations.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); // Bottom wall rotation
    planeRotations.push_back(glm::vec3(90.0f, 0.0f, 0.0f)); // Back wall rotation

    std::vector<glm::vec3> planeScales;
    planeScales.push_back(glm::vec3(2.0f, 1.0f, 15.0f)); // Left wall scale
    planeScales.push_back(glm::vec3(2.0f, 1.0f, 15.0f)); // Right wall scale
    planeScales.push_back(glm::vec3(2.0f, 1.0f, 15.0f)); // Up wall scale
    planeScales.push_back(glm::vec3(2.0f, 1.0f, 15.0f)); // Bottom wall scale
    planeScales.push_back(glm::vec3(2.0f, 1.0f, 2.0f)); // Back wall scale

    std::vector<glm::vec3> planeUVScale;
    for (int i = 0; i < planeScales.size(); i++) 
    {
        planeUVScale.push_back(glm::vec3(planeScales[i].x, planeScales[i].z, 1.0f));
    }

    // Point Lights
    std::vector<glm::vec3> pointLightPositions;
    pointLightPositions.push_back(glm::vec3(0.7f, 0.2f, 2.0f));
    pointLightPositions.push_back(glm::vec3(2.3f, -3.3f, -4.0f));
    pointLightPositions.push_back(glm::vec3(-4.0f, 2.0f, -12.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -15.0f));

    std::vector<glm::vec3> pointLightColors;
    pointLightColors.push_back(glm::vec3(0.8f, 0.2f, 0.0f));
    pointLightColors.push_back(glm::vec3(0.3f, 0.8f, 0.0f));
    pointLightColors.push_back(glm::vec3(0.1f, 0.6f, 0.8f));
    pointLightColors.push_back(glm::vec3(0.2f, 0.7f, 0.5f));
    pointLightColors.push_back(glm::vec3(100.8f, 100.1f, 100.2f));

    //// Directional Light direction
    //std::vector<glm::vec3> directionalLightDirection;
    //glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - glm::vec3(1.0f, 1.0f, 1.0f));
    //directionalLightDirection.push_back(lightDirection);

    //std::vector<glm::vec3> lightColor;
    //lightColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    sceneAttributes["planePositions"] = planePositions;
    sceneAttributes["planeRotations"] = planeRotations;
    sceneAttributes["planeScales"] = planeScales;
    sceneAttributes["planeUVScale"] = planeUVScale;
    sceneAttributes["pointLightPositions"] = pointLightPositions;
    sceneAttributes["pointLightColors"] = pointLightColors;
    /*sceneAttributes["directionalLightDirection"] = directionalLightDirection;
    sceneAttributes["lightColor"] = lightColor;*/
}

void HDRMappingTestScene::RenderScene(unsigned int deferredQuadFrameBuffer)
{

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
        UseShaderProgram(HDRMappingShaderProgramName);
        std::shared_ptr<Shader> HDRLightingMappingShaderProgram = GetShaderProgram(HDRMappingShaderProgramName);

        // VS stage Uniform inputs
        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
        // You can set the uniforms once or every frame
        // View
        HDRLightingMappingShaderProgram->setMat4("view", GetCamera("MainCamera")->GetViewMatrix());
        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        HDRLightingMappingShaderProgram->setMat4("projection", projection);

        // lightSpaceMatrix
        // Light Space Projection Matrix
        //float near_plane = 1.0f, far_plane = 7.5f;
        //glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        //// Light Space View Matrix
        //glm::mat4 lightView = glm::lookAt(sceneAttributes["directionalLightPosition"][0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        //glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        //HDRLightingMappingShaderProgram->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // FS stage Uniform inputs
        HDRLightingMappingShaderProgram->setVec3("viewPos", GetCamera("MainCamera")->Position);

        //HDRLightingMappingShaderProgram->setFloat("material.shininess", 32.0f);

        //HDRLightingMappingShaderProgram->setFloat("height_scale", 0.1f);

        //DIRECTIONAL LIGHT
        //HDRLightingMappingShaderProgram->setVec3("lightDir", sceneAttributes["directionalLightDirection"][0]);

        // POINT LIGHT
        for (int i = 0; i < sceneAttributes["pointLightPositions"].size(); i++)
        {
            std::string lightPosVariableNameInShader = "lights[" + std::to_string(i) + "].Position";
            std::string lightColorVariableNameInShader = "lights[" + std::to_string(i) + "].Color";
            HDRLightingMappingShaderProgram->setVec3(lightPosVariableNameInShader, sceneAttributes["pointLightPositions"][i]);
            HDRLightingMappingShaderProgram->setVec3(lightColorVariableNameInShader, sceneAttributes["pointLightColors"][i]);
        }
        
        // Planes VAO
        // Set the material diffuse and specular maps
        // texture1 - Material diffuse
        // texture2 - Material specular 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GetTextureID(WallDiffuseMap));
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, GetTextureID(WallNormalMap));
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, GetTextureID(WallDisplacementMap));

        for (int i = 0; i < sceneAttributes["planePositions"].size(); i++)
        {
            // Plane
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 translate = glm::mat4(1.0f);
            glm::mat4 rotate = glm::mat4(1.0f);
            glm::mat4 scale = glm::mat4(1.0f);

            translate = glm::translate(translate, sceneAttributes["planePositions"][i]); // translate it down so it's at the center of the scene

            glm::vec3 rotation(sceneAttributes["planeRotations"][i]);
            rotate = glm::rotate(rotate, glm::radians(rotation.z), glm::vec3(0, 0, 1));
            rotate = glm::rotate(rotate, glm::radians(rotation.y), glm::vec3(0, 1, 0));
            rotate = glm::rotate(rotate, glm::radians(rotation.x), glm::vec3(1, 0, 0));

            glm::vec3 planeScale(sceneAttributes["planeScales"][i]);
            scale = glm::scale(scale, planeScale);	// it's a bit too big for our scene, so scale it down
            
            model = translate * rotate * scale;

            HDRLightingMappingShaderProgram->setFloat("UV_scaleX", sceneAttributes["planeUVScale"][i].x);
            HDRLightingMappingShaderProgram->setFloat("UV_scaleY", sceneAttributes["planeUVScale"][i].y);

            HDRLightingMappingShaderProgram->setMat4("model", model);
            HDRLightingMappingShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            DrawMesh("plane", HDRMappingShaderProgramName);
        }
    }
}