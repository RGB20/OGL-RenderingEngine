#include <headers/ShaderHandler.h>
#include "headers/WindowManager.h"

// Timing
#include <chrono>

// Scenese
#include "headers/Scenes/StensilTestScene/SensilTestScene.h"
#include "headers/Scenes/LightTestingScene/LightingTestScene.h"
#include "headers/Scenes/DepthTestingScene/DepthTestingScene.h"
#include "headers/Scenes/BlendingTestingScene/BlendingTestScene.h"
#include "headers/Scenes/SkyboxTestingScene/SkyboxTestScene.h"
#include "headers/Scenes/GeometryShaderTestingScene/GeometryShaderTestScene.h"
#include "headers/Scenes/InstancingTestingScene/InstancingTestScene.h"
#include "headers/Scenes/ShadowMappingTestScene/ShadowMappingTestScene.h"
#include "headers/Scenes/ShadowMappingMegaScene/ShadowMappingMegaScene.h"
#include "headers/Scenes/NormalMappingTestScene/NormalMappingTestScene.h"
#include "headers/Scenes/ParallaxMappingTestScene/ParallaxMappingTestScene.h"
#include "headers/Scenes/HDRMappingTestScene/HDRMappingTestScene.h"
#include "headers/Scenes/DemoTestScene/DemoTestScene.h"

// These functions are defined in the Utilities header file
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

std::chrono::duration<double> deltaTime;

SceneManager sceneManager = SceneManager();
std::string activeScene;
int main()
{
    WindowManager windowManager(4, 1, SCR_WIDTH, SCR_HEIGHT);
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
    
    // configure MSAA framebuffer and Render buffers for off screen rendering of the scenes [MULTI SAMNPLING]
    // --------------------------
    unsigned int MSAAframebuffer;
    glGenFramebuffers(1, &MSAAframebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, MSAAframebuffer);
    // create a multisampled color attachment texture, it's also HDR enabled
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::MSAA FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Reset the frame buffer binding to default frame buffer (good practice)

    // Setup normal framebuffer and texture for blitting MSAA framebuffer into [NON MULTISAMPLED]
    // --------------------------
    unsigned int IntermediateFrameBufferObject;
    glGenFramebuffers(1, &IntermediateFrameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, IntermediateFrameBufferObject);

    // Setup texture for rendering the frame buffer to (Color attachment 0), This will be the input texture for quad rendering. It is enabled for HDR content
    unsigned int colorBufferTexture;
    glGenTextures(1, &colorBufferTexture);
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture, 0);	// we only need a color buffer

    // Check if the frame buffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::NON MSAA FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create the Quad Scene for rendering to with the color buffer attachment blitted to by the MSAA frame buffer
    // Shader program for the Quad scene
    Scene quadScene = Scene();
    std::string quadShaderProgramName = "quadShaderProgram";

    std::unordered_map<SHADER_TYPES, std::string> quadVertexShaders;

    quadVertexShaders[SHADER_TYPES::VERTEX_SHADER] = GetCurrentDir() + "\\shaders\\quadVertexShader.vs";
    quadVertexShaders[SHADER_TYPES::FRAGMENT_SHADER] = GetCurrentDir() + "\\shaders\\quadFragmentShader.fs";
    quadScene.AddShader(quadShaderProgramName, quadVertexShaders);
    quadScene.GetShaderProgram(quadShaderProgramName)->setInt("frameBufferHDRColorAttachment", 0);

    // Add/Load Models
    quadScene.AddPresetMesh("quad", DEFAULT_MESHES::QUAD);

    // --------------------------------------------- Test Scenes ---------------------------------------------
    // Load All Other Scenes
    sceneManager.RegisterScene("StensilTestScene", std::make_shared<StensilTestScene>());
    sceneManager.RegisterScene("LightTestingScene", std::make_shared<LightingTestScene>());
    sceneManager.RegisterScene("DepthTestingScene", std::make_shared<DepthTestingScene>());
    sceneManager.RegisterScene("BlendingTestingScene", std::make_shared<BlendingTestScene>());
    sceneManager.RegisterScene("SkyboxTestingScene", std::make_shared<SkyboxTestScene>());
    sceneManager.RegisterScene("GeometryShaderTestingScene", std::make_shared<GeometryShaderTestScene>());
    sceneManager.RegisterScene("InstancingTestingScene", std::make_shared<InstancingTestScene>());
    sceneManager.RegisterScene("ShadowMappingTestScene", std::make_shared<ShadowMappingTestScene>());
    sceneManager.RegisterScene("ShadowMappingMegaScene", std::make_shared<ShadowMappingMegaScene>());
    sceneManager.RegisterScene("NormalMappingTestScene", std::make_shared<NormalMappingTestScene>());
    sceneManager.RegisterScene("ParallaxMappingTestScene", std::make_shared<ParallaxMappingTestScene>());
    sceneManager.RegisterScene("HDRMappingTestScene", std::make_shared<HDRMappingTestScene>());
    sceneManager.RegisterScene("DemoTestScene", std::make_shared<DemoTestScene>());

    activeScene = "DemoTestScene";
    sceneManager.Scenes[activeScene]->SetupScene();

    // Timing calculation
    // Store the time of the previous frame/update
    auto previousTime = std::chrono::high_resolution_clock::now();
    
    // render loop
    while (!glfwWindowShouldClose(windowManager.GetWindow()))
    {
        // Get the current time
        auto currentTime = std::chrono::high_resolution_clock::now();

        // Calculate the duration between the current and previous time
        deltaTime = currentTime - previousTime;
        
        // Update previousTime for the next iteration
        previousTime = currentTime;

        int FPS = 1 / deltaTime.count();

        glm::vec3 camPos = sceneManager.Scenes[activeScene]->Cameras["MainCamera"]->Position;
        std::string camPosStr = "Cam Pos : (" + std::to_string(camPos.x) + ", " + std::to_string(camPos.y) + ", " + std::to_string(camPos.z) + ")";
        auto windowTitle = "LearnOpenGL FPS : " + std::to_string(FPS) + " " + camPosStr;
        windowManager.UpdateWindowTitle(windowTitle);

        // input
        // -----
        processInput(windowManager.GetWindow());

        // 1. Render scene as normal in multisampled buffers
        glBindFramebuffer(GL_FRAMEBUFFER, MSAAframebuffer);
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        
        if (activeScene == "ShadowMappingTestScene"     ||
            activeScene == "NormalMappingTestScene"     || 
            activeScene == "ShadowMappingMegaScene"     ||
            activeScene == "ParallaxMappingTestScene"   ||
            activeScene == "HDRMappingTestScene"        ||
            activeScene == "DemoTestScene")
        {
            sceneManager.Scenes[activeScene]->RenderScene(MSAAframebuffer);
        }
        else 
        {
            sceneManager.Scenes[activeScene]->RenderScene();
        }

        // 2. Now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAAframebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, IntermediateFrameBufferObject);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // 3 : Render a Quad to the default frame buffer reading from the color texture blitted to from the Multi-Sampled texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default draw frame buffer
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        quadScene.UseShaderProgram(quadShaderProgramName);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBufferTexture);

        quadScene.GetShaderProgram(quadShaderProgramName)->setFloat("exposure", 1.0f);

        quadScene.DrawMesh("quad", quadShaderProgramName);

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

    float movementScale = 10 * deltaTime.count();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(FORWARD, movementScale);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(BACKWARD, movementScale);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(LEFT, movementScale);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        sceneManager.Scenes[activeScene]->GetCamera("MainCamera")->ProcessKeyboard(RIGHT, movementScale);

    // ShadowMap Demo
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (activeScene == "ShadowMappingMegaScene") {
            sceneManager.Scenes[activeScene]->DemoKeyPressed(GLFW_KEY_T);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        if (activeScene == "ShadowMappingMegaScene") {
            sceneManager.Scenes[activeScene]->DemoKeyPressed(GLFW_KEY_Y);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        if (activeScene == "ShadowMappingMegaScene") {
            sceneManager.Scenes[activeScene]->DemoKeyPressed(GLFW_KEY_U);
        }
    }
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