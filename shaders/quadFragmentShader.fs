#version 330 core

uniform sampler2D frameBufferHDRColorAttachment;

out vec4 FragColor;

in vec2 TexCoords;

uniform float exposure;

void main()
{   
    const float gamma = 2.2f;
    vec3 hdrColor = texture(frameBufferHDRColorAttachment, TexCoords).rgb;
  
    // reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}