#include <ShaderHandler.h>
#include "WindowManager.h"
#include "SensilTestScene.h"
#include "LightingTestScene.h"
#include "DepthTestingScene.h"
#include "BlendingTestScene.h"

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
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glfwSetFramebufferSizeCallback(windowManager.GetWindow(), framebuffer_size_callback);
    glfwSetInputMode(windowManager.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(windowManager.GetWindow(), mouse_callback);
    glfwSetScrollCallback(windowManager.GetWindow(), scroll_callback);

    sceneManager.RegisterScene("StensilTestScene", std::make_shared<StensilTestScene>());
    sceneManager.RegisterScene("LightTestingScene", std::make_shared<LightingTestScene>());
    sceneManager.RegisterScene("DepthTestingScene", std::make_shared<DepthTestingScene>());
    sceneManager.RegisterScene("BlendingTestingScene", std::make_shared<BlendingTestScene>());
    
    activeScene = "BlendingTestingScene";

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

        sceneManager.Scenes[activeScene]->RenderScene();
        
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