#version 330 core

in vec2 TexCoords;
in vec3 WorldPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform float time;
uniform sampler2D heightMap;
uniform float heightScale;
uniform float waterLevel;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main()
{
    // Use directional vectors and noise to break up the grid-like "square" wave pattern
    vec2 p = WorldPos.xz * 0.15;
    float n = noise(p * 0.5 + time * 0.2);
    float waveA = sin(dot(p, vec2(1.0, 0.6)) + time * 1.5 + n * 3.0);
    float waveB = sin(dot(p, vec2(-0.8, 0.4)) - time * 1.3 + n * 2.0);
    float waveC = sin(dot(p, vec2(0.2, -1.0)) + time * 0.8);
    float ripple = (waveA + waveB + waveC) * 0.33;

    // Sample terrain depth for shore effects
    // Removed * heightScale because the texture already contains world-space height values
    float terrainH = texture(heightMap, TexCoords).r;
    float depth = waterLevel - terrainH;
    
    // Steady rise and fall (spatially asynchronous tide)
    float tideSpeed = 0.7;
    float tideHeight = 1.3;
    
    // Use low-frequency noise to offset the tide phase spatially
    float localPhase = time * tideSpeed + noise(WorldPos.xz * 0.01) * 10.0;
    float tide = sin(localPhase) * tideHeight;
    
    // Effectively shifts the shoreline inland and back out periodically
    float adjDepth = depth - tide;
    float risingFactor = clamp(cos(localPhase), 0.0, 1.0);

    // Random light foam patches that occur near the shoreline only when water level rises
    float foamNoise = noise(WorldPos.xz * 0.15 + time * 0.2);
    float risingFoam = risingFactor * foamNoise * smoothstep(2.5, 0.0, adjDepth) * 0.6;
    
    // Updated shore fade and foam using adjusted depth for the "rising" effect
    float shoreFade = smoothstep(-0.5, 4.0, adjDepth);
    float staticFoam = smoothstep(1.2, 0.0, adjDepth);
    
    ripple *= shoreFade;

    vec3 normal = normalize(vec3(-waveA * 0.15, 1.0, -waveB * 0.15));

    vec3 shallow = vec3(0.18, 0.48, 0.52);
    vec3 deep = vec3(0.04, 0.19, 0.30);
    vec3 waterColor = mix(deep, shallow, 0.5 + ripple * 0.3);
    
    // Apply static shore foam and dynamic rising tide foam
    waterColor = mix(waterColor, vec3(0.95, 0.98, 1.0), max(staticFoam * 0.4, risingFoam));

    vec3 viewDir = normalize(viewPos - WorldPos);
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 4.0);
    // Slightly reduced fresnel intensity (0.4) to keep the deep blue colors visible at distances
    waterColor = mix(waterColor, vec3(0.8, 0.9, 1.0), fresnel * 0.4 * shoreFade);

    // Fade alpha near shore and boost where rising foam is present
    float alpha = clamp(mix(0.35, 0.8, shoreFade) + risingFoam, 0.0, 1.0);
    FragColor = vec4(waterColor, alpha);
}
