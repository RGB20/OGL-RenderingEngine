#version 410 core

in vec3 fNormal;
in vec2 fuv;
in float fHeight;
in vec3 fFragpos;

uniform sampler2D heightMap;

out vec4 FragColor;

uniform float angleAroundCenter;

void main()
{
    float radius = 16000;
    float x = radius * cos(angleAroundCenter);
    float z = radius * sin(angleAroundCenter);
    vec3 lightPos = vec3(x, 2000, z);
    vec3 lightColor = vec3(0.8,0.6,0.7);
    
    vec3 color = vec3(0.4,0.2,0.1);
    vec3 normal = normalize(fNormal);
    // ambient
    vec3 ambient = color;
    // lighting
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 1; i++)
    {
        // diffuse
        vec3 lightDir = normalize(lightPos - fFragpos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = lightColor * diff * color;      
        vec3 result = diffuse;        
        // attenuation (use quadratic as we have gamma correction)
        //float distance = length(fFragpos - lightPos);
        //result *= 1.0 / (distance * distance);
        lighting += result;
    }

    FragColor = vec4(lighting, 1.0f);

    //FragColor = vec4(normal, 1.0f);
}