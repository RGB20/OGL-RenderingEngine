#version 330 core

uniform sampler2D frameBufferColorAttachment;

out vec4 FragColor;

in vec2 TexCoords;

void main()
{   
    FragColor = texture(frameBufferColorAttachment, TexCoords);
}