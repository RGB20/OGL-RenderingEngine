#include <ShaderHandler.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "Camera.h"
#include "Utilities.h"

// These functions are defined in the Utilities header file
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

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

float planeVertices[] = {
     // Position        // Normal       
     0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
    -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // top left 
};
unsigned int planeIndices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

float cubeVertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};


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
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // -----------------Common--------------------------------------------------------------------------- //
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // -------------------------------------------------------------------------------------------------- //
    
    // Object shader program and other handlers
    std::string objectVertexShaderPath = GetCurrentDir() + "\\shaders\\objectVertexShader.vs";
    std::string objectFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectFragmentShader.fs";
    Shader objectShaderProgram(objectVertexShaderPath.c_str(), objectFragmentShaderPath.c_str());
    unsigned int texture1, texture2;
    unsigned int objectVAO;
    //glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

    {
        // Load the TEXTURES
        std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\containerDiffuseMap.png";
        texture1 = loadTexture(containerDiffuseTexMap.c_str());

        std::string containerSpecTexMap = GetCurrentDir() + "\\textures\\containerSpecularMap.png";
        texture2 = loadTexture(containerSpecTexMap.c_str());

        objectShaderProgram.use();
        // We need to now tell OpenGL for each sampler which texture unit it belongs to
        objectShaderProgram.setInt("material.diffuse", 0);
        objectShaderProgram.setInt("material.specular", 1);

        // VAO
        // objectVAO : Creating a Vertex Attribute Object (objectVAO) to store the attribute state
        //unsigned int objectVAO;
        glGenVertexArrays(1, &objectVAO);

        glBindVertexArray(objectVAO);

        // VBO
        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        // EBO
        //unsigned int EBO;
        //glGenBuffers(1, &EBO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

        // Lets define how OpenGL should intrepret the vertex attribute forom the last bound buffer.
        // Which in our case is the VBO containing the vertex positions interleaved with color.
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // UV's
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Set object color uniform
        //objectShaderProgram.setVec3("objectColor", objectColor);

        glUseProgram(0);
    }

    // -------------------------------------------------------------------------------------------------- //

    // Light shader program and other handlers
    std::string lightVertexShaderPath = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    std::string lightFragmentShaderPath = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    Shader lightShaderProgram(lightVertexShaderPath.c_str(), lightFragmentShaderPath.c_str());
    unsigned int lightVAO;
    //glm::vec3 lightColor(1.0f, 0.5f, 0.31f);
    glm::vec3 lightPosition(1.2f, 1.0f, 5.0f);
    glm::vec3 lightDirection(-0.2f, -1.0f, -0.3f);

    {
        lightShaderProgram.use();
        // We need to now tell OpenGL for each sampler which texture unit it belongs to
        //objectShaderProgram.setInt("texture1", 0);
        //objectShaderProgram.setInt("texture2", 1);

        // objectVAO : Creating a Vertex Attribute Object (objectVAO) to store the attribute state
        //unsigned int objectVAO;
        glGenVertexArrays(1, &lightVAO);

        glBindVertexArray(lightVAO);

        // VBO
        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        // EBO
        //unsigned int EBO;
        //glGenBuffers(1, &EBO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // Lets define how OpenGL should intrepret the vertex attribute forom the last bound buffer.
        // Which in our case is the VBO containing the vertex positions interleaved with color.
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glUseProgram(0);
    }

    // light Model transformations
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPosition);
    lightModel = glm::scale(lightModel, glm::vec3(0.2f));

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

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // RENDER OBJECTS
        objectShaderProgram.use();

        // Set light color uniform
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        //lightColor.x = sin(glfwGetTime() * 2.0f);
        //lightColor.y = sin(glfwGetTime() * 0.7f);
        //lightColor.z = sin(glfwGetTime() * 1.3f);
        objectShaderProgram.setVec3("lightColor", lightColor);
        //objectShaderProgram.setVec3("objectColor", objectColor);
        objectShaderProgram.setVec3("viewPos", camera.Position);

        // You can set the uniforms once or every frame
        // View
        objectShaderProgram.setMat4("view", camera.GetViewMatrix());

        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        objectShaderProgram.setMat4("projection", projection);

        //objectShaderProgram.setVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
        //objectShaderProgram.setVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
        objectShaderProgram.setVec3("material.specular", glm::vec3(0.0f, 0.0f, 0.0f));
        objectShaderProgram.setFloat("material.shininess", 32.0f);

        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
        objectShaderProgram.setVec3("light.position", camera.Position);
        objectShaderProgram.setVec3("light.direction", camera.Front);
        objectShaderProgram.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        objectShaderProgram.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

        objectShaderProgram.setVec3("light.ambient", ambientColor);
        objectShaderProgram.setVec3("light.diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram.setVec3("light.specular",glm::vec3( 1.0f, 1.0f, 1.0f));

        objectShaderProgram.setFloat("light.constant", 1.0f);
        objectShaderProgram.setFloat("light.linear", 0.09f);
        objectShaderProgram.setFloat("light.quadratic", 0.032f);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(objectVAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 objectModel;
            objectModel = glm::mat4(1.0f);
            objectModel = glm::translate(objectModel, cubePositions[i]);
            float angle = 20.0f * i;
            objectModel = glm::rotate(objectModel, currentFrame * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            // Set the uniform model matrix
            objectShaderProgram.setMat4("model", objectModel);
            // Set the inverse model matrix
            objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(objectModel))));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        // Set the uniform model matrix
        //glm::mat4 objectModel = glm::mat4(1.0f);
        //objectModel = glm::translate(objectModel, glm::vec3(0.0f, 0.0f, -5.0f));
        //objectModel = glm::scale(objectModel, glm::vec3(10.0f, 10.0f, 10.0f));
        //objectShaderProgram.setMat4("model", objectModel);
        //// Set the inverse model matrix
        //objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(objectModel))));

        // Bind the texture
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // RENDER LIGHTS
        lightShaderProgram.use();
        // Set the transform uniforms once or every frame
        lightShaderProgram.setMat4("model", lightModel);
        lightShaderProgram.setMat4("view", camera.GetViewMatrix());
        lightShaderProgram.setMat4("projection", projection);

        // Set light color uniform
        lightShaderProgram.setVec3("lightColor", lightColor);
        
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}