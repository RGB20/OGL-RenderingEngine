#version 330 core

in vec2 TexCoords;
in vec3 WorldPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform float time;

void main()
{
    float rippleA = sin((WorldPos.x * 0.045) + time * 0.9);
    float rippleB = sin((WorldPos.z * 0.038) - time * 0.7);
    float ripple = (rippleA + rippleB) * 0.5;

    vec3 shallow = vec3(0.18, 0.48, 0.52);
    vec3 deep = vec3(0.04, 0.19, 0.30);
    vec3 waterColor = mix(deep, shallow, 0.42 + ripple * 0.08);

    vec3 viewDir = normalize(viewPos - WorldPos);
    float fresnel = pow(1.0 - max(viewDir.y, 0.0), 3.0);
    waterColor = mix(waterColor, vec3(0.60, 0.82, 0.86), fresnel * 0.35);

    FragColor = vec4(waterColor, 0.58);
}
