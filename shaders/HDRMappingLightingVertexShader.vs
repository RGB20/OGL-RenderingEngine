#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} vs_out;

uniform mat4 model;
uniform mat3 modelInvT;
uniform mat4 view;
uniform mat4 projection;

uniform float UV_scaleX;
uniform float UV_scaleY;

void main()
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
    
    vec2 scaledUVs = aTexCoords;
    scaledUVs.x *= UV_scaleX; 
    scaledUVs.y *= UV_scaleY;
    vs_out.TexCoords = scaledUVs;

    vs_out.Normal = normalize(modelInvT * aNormal);
};