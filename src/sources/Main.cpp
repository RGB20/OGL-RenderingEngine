#include <headers/ShaderHandler.h>
#include "headers/WindowManager.h"
#include "headers/SensilTestScene.h"
#include "headers/LightingTestScene.h"
#include "headers/DepthTestingScene.h"
#include "headers/BlendingTestScene.h"
#include "headers/SkyboxTestScene.h"
#include "headers/GeometryShaderTestScene.h"
#include "headers/InstancingTestScene.h"

// These functions are defined in the Utilities header file
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

SceneManager sceneManager = SceneManager();
std::string activeScene;
int main()
{
    WindowManager windowManager(3, 3, SCR_WIDTH, SCR_HEIGHT);
    if (windowManager.InitializeWindow() != true) 
    {
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(windowManager.GetWindow(), framebuffer_size_callback);
    glfwSetInputMode(windowManager.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(windowManager.GetWindow(), mouse_callback);
    glfwSetScrollCallback(windowManager.GetWindow(), scroll_callback);

    // ----------------------------- FRAME BUFFER & RENDER BUFFER -----------------------------
    // Setup Frame buffers and Render buffers for off screen rendering of the scenes
    unsigned int frameBufferObject;
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    // Setup texture for rendering the frame buffer to (Color attachment 0)
    unsigned int colorBufferTexture;
    glGenTextures(1, &colorBufferTexture);
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the color bufffer texture to the currently bound frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture, 0);

    // We will create a render buffer for the depth and stensil attachment
    // The depth attachment will have 24 bit precision and stensil attachment will have 8 bit precision.
    // Both the depth and stensil buffers are combined into one render buffer object
    unsigned int depthAndStensilRenderBuffer;
    glGenRenderbuffers(1, &depthAndStensilRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthAndStensilRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    // Attach the depth and stensil render buffer to the off screen frame buffer we created
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthAndStensilRenderBuffer);

    // Check if the frame buffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create the Quad Scene for rendering to with the color buffer attachment
    // Shader program for the Quad scene
    Scene quadScene = Scene();
    std::string quadShaderProgramName = "quadShaderProgram";

    std::unordered_map<SHADER_TYPES, std::string> quadVertexShaders;

    quadVertexShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\quadVertexShader.vs";
    quadVertexShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\quadFragmentShader.fs";
    quadScene.AddShader(quadShaderProgramName, quadVertexShaders);
    quadScene.GetShaderProgram(quadShaderProgramName)->setInt("frameBufferColorAttachment", 0);

    // Add/Load Models
    quadScene.AddPresetModels("quad", DEFAULT_MODELS::QUAD);

    // --------------------------------------------- Test Scenes ---------------------------------------------
    // Load All Other Scenes
    sceneManager.RegisterScene("StensilTestScene", std::make_shared<StensilTestScene>());
    sceneManager.RegisterScene("LightTestingScene", std::make_shared<LightingTestScene>());
    sceneManager.RegisterScene("DepthTestingScene", std::make_shared<DepthTestingScene>());
    sceneManager.RegisterScene("BlendingTestingScene", std::make_shared<BlendingTestScene>());
    sceneManager.RegisterScene("SkyboxTestingScene", std::make_shared<SkyboxTestScene>());
    sceneManager.RegisterScene("GeometryShaderTestingScene", std::make_shared<GeometryShaderTestScene>());
    sceneManager.RegisterScene("InstancingTestingScene", std::make_shared<InstancingTestScene>());
    
    activeScene = "InstancingTestingScene";

    sceneManager.Scenes[activeScene]->SetupScene();

    // render loop
    while (!glfwWindowShouldClose(windowManager.GetWindow()))
    {
        //// per-frame time logic
        //// --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(windowManager.GetWindow());

        // First : Render the scene to the frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        sceneManager.Scenes[activeScene]->RenderScene();

        // Second : Render a Quad to the default frame buffer reding from the color texture output to which the scene was rendered to
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        quadScene.UseShaderProgram(quadShaderProgramName);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
        quadScene.DrawModel("quad", quadShaderProgramName);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(windowManager.GetWindow());
        glfwPollEvents();
    }

    //glfwTerminate();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(RIGHT, deltaTime);
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

    sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessMouseScroll(static_cast<float>(yoffset));
}