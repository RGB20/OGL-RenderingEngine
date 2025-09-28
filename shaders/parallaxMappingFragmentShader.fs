#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

// Object Material
struct Material {
    sampler2D diffuseMap;
    sampler2D normalMap;
    sampler2D depthMap;
    float shininess;
};
uniform Material material;

uniform float height_scale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(material.depthMap, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return clamp(texCoords - p, 0.0f, 1.0f);    
}

void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir   = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);

     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(material.normalMap, texCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);

    // get diffuse color
    vec3 color = texture(material.diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-fs_in.TangentLightPos, normal);
    vec3 halfwayDir = normalize(lightDir + fs_in.TangentViewPos);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}