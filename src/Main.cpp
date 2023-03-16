#include <ShaderHandler.h>
#include "WindowManager.h"
#include "SensilTestScene.h"

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
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glfwSetFramebufferSizeCallback(windowManager.GetWindow(), framebuffer_size_callback);
    glfwSetInputMode(windowManager.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(windowManager.GetWindow(), mouse_callback);
    glfwSetScrollCallback(windowManager.GetWindow(), scroll_callback);

    sceneManager.RegisterScene("stensilTestScene", std::make_shared<StensilTestScene>());
    
    sceneManager.Scenes["stensilTestScene"]->SetupScene();

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

        sceneManager.Scenes["stensilTestScene"]->RenderScene();
        
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
        sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessKeyboard(RIGHT, deltaTime);
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

    sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    sceneManager.Scenes["stensilTestScene"]->GetCamera("MainCamera")->ProcessMouseScroll(static_cast<float>(yoffset));
}