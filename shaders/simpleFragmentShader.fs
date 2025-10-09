#version 330 core

struct Material {
    sampler2D diffuse;
};
uniform Material material;

out vec4 FragColor;  

uniform bool texturing;
uniform vec3 objectColor;

in vec2 TexCoords;

void main()
{   
    if (texturing == true)
    {
     FragColor = texture(material.diffuse, TexCoords);
    } else
    {
       FragColor = vec4(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f);
    }
}