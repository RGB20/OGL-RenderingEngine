#include "headers/ShaderHandler.h"

Shader::Shader(std::string shaderName, std::unordered_map<SHADER_TYPES, std::string> shaders)// const char* vertexPath, const char* fragmentPath)
{
    this->shaderName = shaderName;
    
    // Load the shaders provided
    std::unordered_map<SHADER_TYPES, std::string> shaderCode;
    
    try
    {
        for (auto shaderType : shaders)
        {
            // Open shader file and read the data
            std::ifstream shaderFileHandle;
            shaderFileHandle.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            shaderFileHandle.open(shaderType.second.c_str());
            std::stringstream shaderFileStream;
            // read file's buffer contents into streams
            shaderFileStream << shaderFileHandle.rdbuf();
            // close file handlers
            shaderFileHandle.close();
            // convert stream into string
            shaderCode[shaderType.first] = shaderFileStream.str();
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    std::vector<unsigned int> shaderIDs;

    for (auto shader : shaderCode)
    {
        shaderIDs.push_back(CreateShader(shader.first, shader.second.c_str()));
    }

    // shader Program
    ID = glCreateProgram();
    for (int i = 0; i < shaderIDs.size(); i++)
    {
        glAttachShader(ID, shaderIDs[i]);
    }
    glLinkProgram(ID);
    // print linking errors if any
    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    for (int i = 0; i < shaderIDs.size(); i++)
    {
        glDeleteShader(shaderIDs[i]);
    }
}

unsigned int Shader::CreateShader(SHADER_TYPES type, const char* shaderCode)
{
    unsigned int shaderID;
    int success;
    char infoLog[512];
    switch(type)
    {
        case(SHADER_TYPES::VERTEX_SHADER) :
            // Vertex Shader
            shaderID = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(shaderID, 1, &shaderCode, NULL);
            glCompileShader(shaderID);
            // print compile errors if any
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            };
            break;

        case(SHADER_TYPES::FRAGMENT_SHADER):
            // Vertex Shader
            shaderID = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(shaderID, 1, &shaderCode, NULL);
            glCompileShader(shaderID);
            // print compile errors if any
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            };
            break;
        
        case(SHADER_TYPES::GEOMETRY_SHADER):
            // Vertex Shader
            shaderID = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(shaderID, 1, &shaderCode, NULL);
            glCompileShader(shaderID);
            // print compile errors if any
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
            };
            break;
    }

    return shaderID;
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, glm::vec3 value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, glm::mat3 value) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
