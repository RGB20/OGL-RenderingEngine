#include <ShaderHandler.h>
#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Utilities.h"
#include "Model.h"


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

float planeVertices[] = {
     // Position         // Normal        // UV     
     5.0f, -0.5f, 5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,  // front right
    -5.0f, -0.5f, 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // front left
    -5.0f, -0.5f,-5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f,  // back left
     5.0f, -0.5f,-5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f  // back right 
};

unsigned int planeIndices[] = {  // note that we start from 0!
    0, 1, 2,   // first triangle
    0, 2, 3    // second triangle
};

float cubeVertices[] = {
    // positions          // Normal            // UV
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // -----------------Common--------------------------------------------------------------------------- //
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // -------------------------------------------------------------------------------------------------- //
    
    // Object shader program and other handlers
    std::string objectVertexShaderPath = GetCurrentDir() + "\\shaders\\objectDepthTesting.vs";
    std::string objectFragmentShaderPath = GetCurrentDir() + "\\shaders\\objectDepthTesting.fs";
    Shader objectShaderProgram(objectVertexShaderPath.c_str(), objectFragmentShaderPath.c_str());
    
    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\";
    unsigned int texture1 = TextureFromFile("containerDiffuseMap.png", containerDiffuseTexMap.c_str());

    std::string wallDiffuseTexMap = GetCurrentDir() + "\\textures\\";
    unsigned int texture2 = TextureFromFile("wall.jpg", containerDiffuseTexMap.c_str());
    
    objectShaderProgram.setInt("material.diffuse", 0);


    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

    // plane EBO
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // load models
    // -----------
    //std::string modelFilePath = GetCurrentDir() + "\\Models\\backpack\\backpack.obj";
    //Model backpackModel(modelFilePath);

    //unsigned int texture1, texture2;
    //unsigned int objectVAO;
    //glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

    //{
    //    // Load the TEXTURES
    //    std::string containerDiffuseTexMap = GetCurrentDir() + "\\textures\\containerDiffuseMap.png";
    //    texture1 = loadTexture(containerDiffuseTexMap.c_str());

    //    std::string containerSpecTexMap = GetCurrentDir() + "\\textures\\containerSpecularMap.png";
    //    texture2 = loadTexture(containerSpecTexMap.c_str());

    //    objectShaderProgram.use();
    //    // We need to now tell OpenGL for each sampler which texture unit it belongs to
    //    objectShaderProgram.setInt("material.diffuse", 0);
    //    objectShaderProgram.setInt("material.specular", 1);

    //    // VAO
    //    // objectVAO : Creating a Vertex Attribute Object (objectVAO) to store the attribute state
    //    //unsigned int objectVAO;
    //    glGenVertexArrays(1, &objectVAO);

    //    glBindVertexArray(objectVAO);

    //    // VBO
    //    unsigned int VBO;
    //    glGenBuffers(1, &VBO);
    //    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    //    // EBO
    //    //unsigned int EBO;
    //    //glGenBuffers(1, &EBO);
    //    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    //    // Lets define how OpenGL should intrepret the vertex attribute forom the last bound buffer.
    //    // Which in our case is the VBO containing the vertex positions interleaved with color.
    //    // position attribute
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    //    glEnableVertexAttribArray(0);
    //    // Normal
    //    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //    glEnableVertexAttribArray(1);
    //    // UV's
    //    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //    glEnableVertexAttribArray(2);

    //    // Set object color uniform
    //    //objectShaderProgram.setVec3("objectColor", objectColor);

    //    glUseProgram(0);
    //}

    // -------------------------------------------------------------------------------------------------- //

    // Light shader program and other handlers
    std::string lightVertexShaderPath = GetCurrentDir() + "\\shaders\\lightVertexShader.vs";
    std::string lightFragmentShaderPath = GetCurrentDir() + "\\shaders\\lightFragmentShader.fs";
    Shader lightShaderProgram(lightVertexShaderPath.c_str(), lightFragmentShaderPath.c_str());
    unsigned int lightVAO;
    //glm::vec3 lightPosition(1.2f, 1.0f, 5.0f);

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // RENDER OBJECTS
        objectShaderProgram.use();

        // VS stage Uniform inputs
        // Uniforms are bound to the shader program and do not care if you access them from the VS of FS stage
        // Saying VS stage Uniform inputs is just a comment to make code easy to read and debug
        // You can set the uniforms once or every frame
        // View
        objectShaderProgram.setMat4("view", camera.GetViewMatrix());
        // Projection
        projection = glm::perspective(glm::radians(ZOOM), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.0f);
        objectShaderProgram.setMat4("projection", projection);

        // FS stage Uniform inputs
        objectShaderProgram.setVec3("viewPos", camera.Position);

        // Set the material diffuse and specular maps
        // texture1 - Material diffuse
        // texture2 - Material specular 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);
        objectShaderProgram.setVec3("material.specular", glm::vec3(0.5,0.5,0.5));
        objectShaderProgram.setFloat("material.shininess", 32.0f);

        //DIRECTIONAL LIGHT
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
        objectShaderProgram.setVec3("dirLight.direction", directionalLightDirection);
        objectShaderProgram.setVec3("dirLight.ambient", ambientColor);
        objectShaderProgram.setVec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // SPOT LIGHT
        objectShaderProgram.setVec3("spotLight.position", camera.Position);
        objectShaderProgram.setVec3("spotLight.direction", camera.Front);
        objectShaderProgram.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        objectShaderProgram.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

        objectShaderProgram.setVec3("spotLight.ambient", ambientColor);
        objectShaderProgram.setVec3("spotLight.diffuse", diffuseColor); // darken diffuse light a bit
        objectShaderProgram.setVec3("spotLight.specular",glm::vec3( 1.0f, 1.0f, 1.0f));

        objectShaderProgram.setFloat("spotLight.constant", 1.0f);
        objectShaderProgram.setFloat("spotLight.linear", 0.09f);
        objectShaderProgram.setFloat("spotLight.quadratic", 0.032f);

        // POINT LIGHT
        // This should be the same as the number of points lights we statically define in the shader
        // TODO: Make this dynamic
        int pointLights = 4;
        for (int i = 0; i < pointLights; ++i)
        {
            std::string number_str = std::to_string(i);
            objectShaderProgram.setVec3("pointLights[" + number_str + "].position", pointLightPositions[i]);
            objectShaderProgram.setFloat("pointLights[" + number_str  + "].constant", 1.0f);
            objectShaderProgram.setFloat("pointLights[" + number_str  + "].linear", 0.09f);
            objectShaderProgram.setFloat("pointLights[" + number_str  + "].quadratic", 0.032f);
            objectShaderProgram.setVec3("pointLights[" + number_str + "].ambient", ambientColor);
            objectShaderProgram.setVec3("pointLights[" + number_str + "].diffuse", diffuseColor); // darken diffuse light a bit
            objectShaderProgram.setVec3("pointLights[" + number_str + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // Cubes VAO
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        {
            // Cube 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[0]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram.setMat4("model", model);
            objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        {
            // Cube 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
            objectShaderProgram.setMat4("model", model);
            objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Planes VAO
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Plane
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[1]); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        objectShaderProgram.setMat4("model", model);
        objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //backpackModel.Draw(objectShaderProgram);

        
        //glBindVertexArray(objectVAO);
        //for (unsigned int i = 0; i < 10; i++)
        //{
        //    glm::mat4 objectModel;
        //    objectModel = glm::mat4(1.0f);
        //    objectModel = glm::translate(objectModel, cubePositions[i]);
        //    float angle = 20.0f * i;
        //    objectModel = glm::rotate(objectModel, currentFrame * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        //    // VS stage Uniform inputs
        //    // Set the uniform model matrix
        //    objectShaderProgram.setMat4("model", objectModel);
        //    // Set the inverse model matrix
        //    objectShaderProgram.setMat3("modelInvT", glm::mat3(glm::transpose(glm::inverse(objectModel))));

        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // RENDER LIGHTS
        lightShaderProgram.use();
        // Set the transform uniforms once or every frame
        lightShaderProgram.setMat4("view", camera.GetViewMatrix());
        lightShaderProgram.setMat4("projection", projection);

        // Set light color uniform
        lightShaderProgram.setVec3("lightColor", lightColor);
        
        glBindVertexArray(lightVAO);
        for (int i = 0; i < pointLights; ++i)
        {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, pointLightPositions[i]);
            lightModel = glm::scale(lightModel, glm::vec3(0.2f));
            lightShaderProgram.setMat4("model", lightModel);
            
            glDrawArrays(GL_TRIANGLES, 0, 36);
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