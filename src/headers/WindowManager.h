#pragma once
#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

class WindowManager {
public:
    WindowManager(unsigned int OGLVersionMajor,
                  unsigned int OGLVersionMinor,
                  unsigned int screenWidth,
                  unsigned int screenHeight) 
        : openGLVersionMajor(OGLVersionMajor), openGLVersionMinor(OGLVersionMinor), SCR_WIDTH(screenWidth), SCR_HEIGHT(screenHeight)
    {}

    ~WindowManager()
    {
        glfwTerminate();
    }

    bool InitializeWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGLVersionMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGLVersionMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4); // Hint to GLFW that we need 4 samples frame buffer for MSAA

        this->window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        return true;
    }

    GLFWwindow* GetWindow() { return window; }

	GLFWwindow* window;
    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;
    unsigned int openGLVersionMajor;
    unsigned int openGLVersionMinor;
};