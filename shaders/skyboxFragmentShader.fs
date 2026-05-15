#version 330 core
out vec4 FragColor;

in vec3 worldDir;

uniform vec3 sunDirection;

float rayleighPhase(float cosTheta)
{
    return 3.0 / (16.0 * 3.14) * (1.0 + cosTheta * cosTheta);
}

float miePhase(float cosTheta, float g)
{
    return 3.0 / (8.0 * 3.14) *
           ((1.0 - g*g) * (1.0 + cosTheta*cosTheta)) /
           ((2.0 + g*g) * pow(1.0 + g*g - 2.0*g*cosTheta, 1.5));
}

vec3 computeSky(vec3 viewDir, vec3 sunDir)
{
    float cosTheta = dot(viewDir, sunDir);

    float rayleigh = rayleighPhase(cosTheta);
    float mie      = miePhase(cosTheta, 0.76);

    vec3 betaR = vec3(5.5e-6, 13.0e-6, 22.4e-6);
    float betaM = 21e-6;

    // Use a density function based on the viewing angle (thicker at horizon)
    float h = max(viewDir.y, 0.0) + 0.15; 
    float atmosphereThickness = 1.0 / h;

    vec3 scattered = atmosphereThickness * (betaR * rayleigh + betaM * mie);

    // Multiply by a large sun intensity/path length factor to make it visible
    return scattered * 40000.0; 
}

void main()
{    
    vec3 viewDir = normalize(worldDir);
    vec3 skyColor = computeSky(viewDir, sunDirection);
    FragColor = vec4(skyColor, 1.0);
}