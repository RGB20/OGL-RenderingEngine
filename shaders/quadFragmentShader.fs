#version 330 core

uniform sampler2D frameBufferColorAttachment;

out vec4 FragColor;

in vec2 TexCoords;

void main()
{   
    float gamma = 1.0f;
    FragColor = texture(frameBufferColorAttachment, TexCoords);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0f/gamma));
}