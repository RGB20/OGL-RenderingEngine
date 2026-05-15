#version 410 core

in vec3 fNormal;
in vec2 fuv;
in float fHeight;
in vec3 fFragpos;

uniform sampler2D heightMap;

out vec4 FragColor;

uniform float timeOfDay01;
uniform vec3 viewPos;
uniform float terrainMinHeight;
uniform float terrainMaxHeight;

float Hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float ValueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = Hash12(i);
    float b = Hash12(i + vec2(1.0, 0.0));
    float c = Hash12(i + vec2(0.0, 1.0));
    float d = Hash12(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

vec3 TerrainAlbedo(float height, vec3 normal)
{
    float heightRange = max(terrainMaxHeight - terrainMinHeight, 0.0001);
    float h = clamp((height - terrainMinHeight) / heightRange, 0.0, 1.0);
    float rawSlope = clamp(1.0 - normal.y, 0.0, 1.0);
    float slope = smoothstep(0.16, 0.72, rawSlope);
    float broadNoise = ValueNoise(fuv * 18.0);
    float patchNoise = broadNoise * 0.55 + ValueNoise(fuv * 62.0) * 0.30 + ValueNoise(fuv * 190.0) * 0.15;

    vec3 grass = vec3(0.34, 0.62, 0.18);
    vec3 sunGrass = vec3(0.56, 0.74, 0.28);
    vec3 deepGrass = vec3(0.16, 0.38, 0.11);
    vec3 dryGrass = vec3(0.58, 0.50, 0.24);
    vec3 dirt = vec3(0.42, 0.31, 0.18);
    vec3 rock = vec3(0.53, 0.51, 0.43);
    vec3 darkRock = vec3(0.28, 0.31, 0.29);
    vec3 paleRock = vec3(0.68, 0.66, 0.56);
    vec3 snow = vec3(0.82, 0.84, 0.78);

    float lowland = 1.0 - smoothstep(0.46, 0.78, h);
    float midShelf = smoothstep(0.18, 0.36, h) * (1.0 - smoothstep(0.72, 0.90, h));
    float flatness = 1.0 - smoothstep(0.22, 0.80, rawSlope);
    float grassElevationMask = 1.0 - smoothstep(96.0, 106.0, height);
    float grassField = clamp(lowland * 0.75 + midShelf * 0.45 + (patchNoise - 0.28) * 0.65, 0.0, 1.0);
    float grassAmount = grassField * mix(0.35, 1.0, flatness) * grassElevationMask;

    float rockAmount = clamp(smoothstep(0.62, 0.88, h) + slope * 0.78 - grassAmount * 0.58, 0.0, 1.0);
    float dirtAmount = smoothstep(0.28, 0.52, h) * (1.0 - smoothstep(0.70, 0.90, h)) * (1.0 - grassAmount * 0.55);
    float snowAmount = smoothstep(0.88, 0.98, h) * (1.0 - slope * 0.45);
    float mossAmount = smoothstep(0.38, 0.70, patchNoise) * (1.0 - smoothstep(0.55, 0.95, rawSlope)) * smoothstep(0.18, 0.52, h) * grassElevationMask;

    vec3 grassColor = mix(deepGrass, grass, smoothstep(0.22, 0.68, patchNoise));
    grassColor = mix(grassColor, sunGrass, smoothstep(0.48, 0.88, patchNoise) * 0.45);
    vec3 lowMix = mix(dirt, dryGrass, smoothstep(0.18, 0.44, h) * 0.45);
    lowMix = mix(lowMix, grassColor, clamp(grassAmount, 0.0, 1.0));

    vec3 rockColor = mix(rock, paleRock, smoothstep(0.35, 0.82, h) * 0.45);
    rockColor = mix(rockColor, darkRock, slope * 0.48);
    rockColor = mix(rockColor, grassColor, mossAmount * 0.50);

    vec3 color = mix(lowMix, dirt, dirtAmount * 0.35);
    color = mix(color, rockColor, rockAmount);
    color = mix(color, snow, clamp(snowAmount, 0.0, 1.0));

    return color * mix(0.82, 1.16, patchNoise);
}

void main()
{
    float radius = 16000;
    float x = radius * cos(timeOfDay01);
    float z = radius * sin(timeOfDay01);
    vec3 lightPos = vec3(x, 3500, z);
    vec3 lightColor = vec3(1.0, 0.84, 0.68);

    vec3 normal = normalize(fNormal);
    vec3 albedo = TerrainAlbedo(fHeight, normal);

    vec3 skyAmbient = vec3(0.28, 0.36, 0.40);
    vec3 groundAmbient = vec3(0.16, 0.12, 0.08);
    float skyVisibility = normal.y * 0.5 + 0.5;
    vec3 ambient = albedo * mix(groundAmbient, skyAmbient, skyVisibility);

    vec3 lightDir = normalize(lightPos - fFragpos);
    float ndotl = dot(normal, lightDir);
    float diffuseTerm = max(ndotl, 0.0);
    float wrappedDiffuse = clamp((ndotl + 0.35) / 1.35, 0.0, 1.0);
    vec3 diffuse = albedo * lightColor * (0.86 * diffuseTerm + 0.14 * wrappedDiffuse);

    vec3 fillDir = normalize(vec3(-lightDir.x, 0.35, -lightDir.z));
    float fillTerm = max(dot(normal, fillDir), 0.0);
    vec3 fill = albedo * vec3(0.16, 0.22, 0.28) * fillTerm * 0.22;

    vec3 viewDir = normalize(viewPos - fFragpos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 96.0);
    vec3 specular = 0.015 * spec * lightColor;

    vec3 color = ambient + diffuse + fill + specular;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luminance), color, 1.22);
    color = max(color - vec3(0.025), vec3(0.0));

    FragColor = vec4(color, 1.0f);
}
