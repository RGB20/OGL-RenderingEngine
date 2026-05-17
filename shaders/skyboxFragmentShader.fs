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
    
    // Normalize sun height (0.0 at horizon, 1.0 at peak elevation approx 0.819)
    float sunHeight = clamp(sunDir.y / 0.819, 0.0, 1.0);
    // Fade out everything when the sun is below the horizon
    float dayFactor = clamp(sunDir.y * 10.0, 0.0, 1.0);

    // Sun Disk Color transition: Red -> Yellow -> White
    vec3 sunDiskColor = mix(vec3(1.0, 0.15, 0.02), vec3(1.0, 0.75, 0.25), smoothstep(0.0, 0.15, sunHeight));
    sunDiskColor = mix(sunDiskColor, vec3(1.0, 1.0, 0.9), smoothstep(0.15, 0.4, sunHeight));

    // Mie scattering (the glow around the sun) transitions from Deep Red -> Gold -> White
    vec3 mieTint = mix(vec3(1.0, 0.25, 0.05), vec3(1.0, 0.75, 0.3), smoothstep(0.0, 0.15, sunHeight));
    mieTint = mix(mieTint, vec3(1.0, 1.0, 1.0), smoothstep(0.15, 0.4, sunHeight));

    // Rayleigh scattering (the background sky): Red/Orange at horizon, Deep Indigo at zenith during sunset
    vec3 sunsetColor = mix(vec3(1.0, 0.35, 0.1), vec3(0.15, 0.1, 0.35), smoothstep(0.0, 0.6, max(viewDir.y, 0.0)));
    
    // Transition Rayleigh from the Sunset gradient to Sky Blue as sun rises
    vec3 rayleighTint = mix(sunsetColor, vec3(0.4, 0.7, 1.0), smoothstep(0.0, 0.25, sunHeight));
    rayleighTint = mix(rayleighTint, vec3(1.0, 1.0, 1.0), smoothstep(0.25, 0.5, sunHeight));

    float rayleigh = rayleighPhase(cosTheta); // Rayleigh scattering remains the same for atmospheric haze
    
    // Pushing 'g' to 0.99 (slightly less than 0.999) allows the colorful halo to be visible
    float mie      = miePhase(cosTheta, 0.99); 

    vec3 betaR = vec3(5.5e-6, 13.0e-6, 22.4e-6);
    // Reduce the Mie coefficient by 90% to simulate a very clear sky with few aerosols
    float betaM = 21e-6 * 0.1; 

    // Use a density function based on the viewing angle (thicker at horizon)
    float h = max(viewDir.y, 0.0) + 0.15; 
    float atmosphereThickness = 1.0 / h;

    // Apply separate tints to Rayleigh and Mie components for a more realistic sky gradient
    vec3 scatteredRayleigh = atmosphereThickness * (betaR * rayleigh) * rayleighTint;
    vec3 scatteredMie = atmosphereThickness * (betaM * mie) * mieTint;
    vec3 scattered = scatteredRayleigh + scatteredMie;
    
    // Tighten the sun disk threshold. 0.99995 results in an angular diameter 
    // of roughly 0.5 degrees, which is the realistic size of the sun from Earth.
    float sunDisk = smoothstep(0.99995, 0.99998, cosTheta);
    vec3 directSun = sunDiskColor * sunDisk * 15000.0;

    // Combine atmospheric scattering with the sharp direct sun disk
    return ((scattered * 40000.0) + directSun) * dayFactor; 
}

void main() 
{    
    vec3 viewDir = normalize(worldDir);
    vec3 skyColor = computeSky(viewDir, sunDirection);
    FragColor = vec4(skyColor, 1.0);
}