#include <ShaderHandler.h>
#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

#include "SceneManager.h"

// These functions are defined in the Utilities header file
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
//unsigned int TextureFromFile(const char* path);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // -----------------Common--------------------------------------------------------------------------- //
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // -------------------------------------------------------------------------------------------------- //
    
    Scene stensilScene;

    std::string objectShaderProgramName = "objectBaseShaderProgram";
    std::string stensilShaderProgramName = "stensilShaderProgram";
    std::string lightShaderProgramName = "lightShaderProgram";

    // Object shader program and other handlers
    std::string objectStensilVertexShaderPath = GetCurrentDir() + "\\shaders\\objectStensilShader.vs";
    std::string objectStensilFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectStensilShader.fs";
    stensilScene.AddShader(stensilShaderProgramName, objectStensilVertexShaderPath, objectStensilFragmentShaderPath);

    std::string lightVertexShaderPath = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    std::string lightFragmentShaderPath = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    stensilScene.AddShader(lightShaderProgramName, lightVertexShaderPath, lightFragmentShaderPath);

    std::string objectVertexShaderPath = GetCurrentDir() + "\\shaders\\objectVertexShader.vs";
    std::string objectFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectFragmentShader.fs";
    stensilScene.AddShader(objectShaderProgramName, objectVertexShaderPath, objectFragmentShaderPath);

    // Load Textures
    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\";
    std::string containerSpecularTexMap = GetCurrentDir() + "\\textures\\";
    std::string wallDiffuseTexMap = GetCurrentDir() + "\\textures\\";

    stensilScene.LoadTexture("containerDiffuseMap", "containerDiffuseMap.png", containerDiffuseTexMap);
    stensilScene.LoadTexture("containerSpecularMap", "containerSpecularMap.png", containerSpecularTexMap);
    stensilScene.LoadTexture("wallDiffuseMap", "wall.jpg", wallDiffuseTexMap);

    stensilScene.GetShaderProgram(objectShaderProgramName)->setInt("material.diffuse", 0);
    stensilScene.GetShaderProgram(objectShaderProgramName)->setInt("material.specular", 1);
    stensilScene.GetShaderProgram(stensilShaderProgramName)->setInt("material.diffuse", 0);

    // Add/Load Models
    stensilScene.AddPresetModels("cube", DEFAULT_MODELS::CUBE);
    stensilScene.AddPresetModels("plane", DEFAULT_MODELS::PLANE);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // RENDERING COMMANDS

        // clear render targets
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // RENDER OBJECTS
        stensilScene.UseShaderProgram(objectShaderProgramName);// objectShaderProgram->use();
        std::shared_ptr<Shader> objectShaderProgram = stensilScene.GetShaderProgram(objectShaderProgramName);

        // VS stage Uniform inputs
        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
        // You can set the uniforms once or every frame
        // View
        objectShaderProgram->setMat4("view", camera.GetViewMatrix());
        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        objectShaderProgram->setMat4("projection", projection);

        // FS stage Uniform inputs
        objectShaderProgram->setVec3("viewPos", camera.Position);

        objectShaderProgram->setFloat("material.shininess", 32.0f);

        //DIRECTIONAL LIGHT
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
        objectShaderProgram->setVec3("dirLight.direction", directionalLightDirection);
        objectShaderProgram->setVec3("dirLight.ambient", ambientColor);
        objectShaderProgram->setVec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // SPOT LIGHT
        objectShaderProgram->setVec3("spotLight.position", camera.Position);
        objectShaderProgram->setVec3("spotLight.direction", camera.Front);
        objectShaderProgram->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        objectShaderProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

        objectShaderProgram->setVec3("spotLight.ambient", ambientColor);
        objectShaderProgram->setVec3("spotLight.diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram->setVec3("spotLight.specular",glm::vec3( 1.0f, 1.0f, 1.0f));

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
            objectShaderProgram->setFloat("pointLights[" + number_str  + "].constant", 1.0f);
            objectShaderProgram->setFloat("pointLights[" + number_str  + "].linear", 0.09f);
            objectShaderProgram->setFloat("pointLights[" + number_str  + "].quadratic", 0.032f);
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
        glBindTexture(GL_TEXTURE_2D, stensilScene.GetTextureID("wallDiffuseMap"));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, stensilScene.GetTextureID("containerSpecularMap"));

        // Plane
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, planePositions[0]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram->setMat4("model", model);
        objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        stensilScene.DrawModel("plane", objectShaderProgramName);
        glBindVertexArray(0);

        // 1st. render pass, draw objects as normal, writing to the stencil buffer
        // --------------------------------------------------------------------
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        // Render the Cubes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stensilScene.GetTextureID("containerDiffuseMap"));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, stensilScene.GetTextureID("containerSpecularMap"));
        {
            // Cube 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[0]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            stensilScene.DrawModel("cube", objectShaderProgramName);
        }
        {
            // Cube 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram->setMat4("model", model);
            objectShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            stensilScene.DrawModel("cube", objectShaderProgramName);
        }

        // 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
        // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
        // the objects' size differences, making it look like borders.
        // -----------------------------------------------------------------------------------------------------------------------------
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        stensilScene.UseShaderProgram(stensilShaderProgramName); //objectStensilShaderProgram.use(); 
        {
            std::shared_ptr<Shader> stensilShaderProgram = stensilScene.GetShaderProgram(stensilShaderProgramName);
            // VS stage Uniform inputs
            // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
            // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
            // You can set the uniforms once or every frame
            // View
            stensilShaderProgram->setMat4("view", camera.GetViewMatrix());
            // Projection
            projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
            stensilShaderProgram->setMat4("projection", projection);

            // FS stage Uniform inputs
            stensilShaderProgram->setVec3("viewPos", camera.Position);

            // Set the material diffuse and specular maps
            // texture1 - Material diffuse
            // texture2 - Material specular 
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, stensilScene.GetTextureID("containerDiffuseMap"));
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
            stensilShaderProgram->setVec3("spotLight.position", camera.Position);
            stensilShaderProgram->setVec3("spotLight.direction", camera.Front);
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
                stensilScene.DrawModel("cube", stensilShaderProgramName);
            }
            {
                // Cube 2
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
                model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// it's a bit too big for our scene, so scale it down
                stensilShaderProgram->setMat4("model", model);
                stensilShaderProgram->setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
                stensilScene.DrawModel("cube", stensilShaderProgramName);
            }
            glBindVertexArray(0);
        }
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);


        // RENDER LIGHTS
        stensilScene.UseShaderProgram(lightShaderProgramName);
        std::shared_ptr<Shader> lightShaderProgram = stensilScene.GetShaderProgram(lightShaderProgramName);
        // Set the transform uniforms once or every frame
        
        lightShaderProgram->setMat4("view", camera.GetViewMatrix());
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
            
            stensilScene.DrawModel("cube", lightShaderProgramName);
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}