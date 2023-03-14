#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include "Utilities.h"
#include "ShaderHandler.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;  // we store the path of the texture to compare with other textures
};

class Mesh {
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(std::shared_ptr<Shader> shader);
private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh();
};