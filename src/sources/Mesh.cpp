#include "headers/Mesh.h"

void Mesh::SetupMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangents
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangents
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BiTangent));
    glBindVertexArray(0);
}

void Mesh::Draw(std::shared_ptr<Shader> shader, bool instancing, uint32_t instancingCount, std::shared_ptr<PatchInfo> patchInfo)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string materialType = textures[i].type;
        std::string name = "";
        if (materialType == "texture_diffuse")
        {
            number = std::to_string(diffuseNr++);
            name = "diffuse";
        }
        else if (materialType == "texture_specular")
        {
            number = std::to_string(specularNr++);
            name = "specular";
        }

        //shader.setInt(("material.diffuse" + number).c_str(), i);
        shader->setInt(("material." + name).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    if (instancing)
    {
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instancingCount);
    }
    else if (patchInfo != nullptr)
    {
        if (patchInfo->patchPrimCount == PATCH_PRIM_TYPE::QUAD_MESH)
            glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
        if (patchInfo->patchPrimCount == PATCH_PRIM_TYPE::TRI_MESH)
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Mesh::LoadMesh(std::string meshName, std::string meshPath)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(meshPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    directory = GetDirFromPath(meshPath);// path.substr(0, path.find_last_of('/\\'));

    processNode(scene->mRootNode, scene);
}

void Mesh::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];
        //meshes.push_back(processMesh(mesh, scene));
        processMesh(mesh, scene);
    }
    // TODO: then do the same for each of its children
    //for (unsigned int i = 0; i < node->mNumChildren; i++)
    //{
    //    processNode(node->mChildren[i], scene);
    //}
}

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0])
        {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        // tangent
        vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

        // biTangent
        vertex.BiTangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);

        //// Calculating the bi tangent but only for triangle meshes
        //if (face.mNumIndices = 3)
        //{
        //    if (mesh->mTextureCoords[0])
        //    {
        //        glm::vec3 pos1 = glm::vec3(mesh->mVertices[face.mIndices[0]].x, mesh->mVertices[face.mIndices[0]].y, mesh->mVertices[face.mIndices[0]].z);
        //        glm::vec3 pos2 = glm::vec3(mesh->mVertices[face.mIndices[1]].x, mesh->mVertices[face.mIndices[1]].y, mesh->mVertices[face.mIndices[1]].z);
        //        glm::vec3 pos3 = glm::vec3(mesh->mVertices[face.mIndices[2]].x, mesh->mVertices[face.mIndices[2]].y, mesh->mVertices[face.mIndices[2]].z);

        //        glm::vec2 uv1 = glm::vec2(mesh->mTextureCoords[0][0].x, mesh->mTextureCoords[0][0].y);
        //        glm::vec2 uv2 = glm::vec2(mesh->mTextureCoords[0][1].x, mesh->mTextureCoords[0][1].y);
        //        glm::vec2 uv3 = glm::vec2(mesh->mTextureCoords[0][2].x, mesh->mTextureCoords[0][2].y);

        //        glm::vec3 edge1 = pos2 - pos1;
        //        glm::vec3 edge2 = pos3 - pos1;

        //        glm::vec2 deltaUV1 = uv2 - uv1;
        //        glm::vec2 deltaUV2 = uv3 - uv1;

        //        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        //        glm::vec3 tangent1, bitangent1;

        //        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        //        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        //        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        //        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        //        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        //        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        //    }
        //}
    }
    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    SetupMesh(vertices, indices, textures);
}

std::vector<Texture> Mesh::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}
// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int TextureFromFile(const char* fileName, const std::string& directory, bool HDR)
{
    std::string filename = std::string(fileName);
    filename = directory + '\\' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum layout;
        GLenum format;
        if (nrComponents == 1)
        {
            layout = GL_RED;
            format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            layout = GL_RGB;
            if (HDR == true) 
            {
                format = GL_SRGB;
            }
            else {
                format = layout;
            }
        }
        else if (nrComponents == 4)
        {
            layout = GL_RGBA;
            if (HDR == true)
            {
                format = GL_SRGB_ALPHA;
            }
            else {
                format = layout;
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, layout, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << fileName << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// utility function for loading a cubemap from file
// ---------------------------------------------------
unsigned int LoadCubeMapFromFile(std::vector<std::string> faces, std::string textureDir, bool HDR)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::string faceFilePath = textureDir + faces[i];
        unsigned char* data = stbi_load(faceFilePath.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum layout;
            GLenum format;
            if (nrChannels == 1)
            {
                layout = GL_RED;
                format = GL_RED;
            }
            else if (nrChannels == 3)
            {
                layout = GL_RGB;
                if (HDR == true)
                {
                    format = GL_SRGB;
                }
                else {
                    format = layout;
                }
            }
            else if (nrChannels == 4)
            {
                layout = GL_RGBA;
                if (HDR == true)
                {
                    format = GL_SRGB_ALPHA;
                }
                else {
                    format = layout;
                }
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, layout, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}