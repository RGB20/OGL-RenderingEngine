#version 330 core

// Object Material
struct Material {
    sampler2D diffuse;
    sampler2D normalMap;
    float shininess;
};
uniform Material material;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 T;
    vec3 B;
    vec3 N;
} fs_in;

// Directional Light
uniform vec3 lightPos;

// TODO: Make the number of lights in the scene dynamic

out vec4 FragColor;  

uniform vec3 viewPos;



void main()
{
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(material.normalMap, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = (normal * 2.0 - 1.0);  // this normal is in tangent space
    normal = normalize(fs_in.TBN * normal);

    // get diffuse color
    vec3 color = texture(material.diffuse, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}