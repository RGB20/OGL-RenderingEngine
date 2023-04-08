#pragma once

#include "Utilities.h"
#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

// Shader types supported by the engine
enum SHADER_TYPES
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    GEOMETRY_SHADER
};

class Shader
{
public:
    // the shader program ID
    unsigned int ID;
    std::string shaderName;

    // constructor reads and builds the shader
    Shader(std::string shaderName, std::unordered_map<SHADER_TYPES, std::string> shaders);// const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setMat3(const std::string& name, glm::mat3 value) const;
    std::string GetShaderName() { return shaderName; }
    unsigned int CreateShader(SHADER_TYPES type, const char* shaderCode);
};