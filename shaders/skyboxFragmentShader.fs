#version 330 core
out vec4 FragColor;

in vec3 worldDir;

uniform vec3 sunDirection;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

vec2 directionToPolarUV(vec3 dir)
{
    dir = normalize(dir);

    float azimuth = atan(dir.z, dir.x);
    float elevation = asin(clamp(dir.y, -1.0, 1.0));

    float u = azimuth / TWO_PI + 0.5;
    float v = elevation / PI + 0.5;

    return vec2(u, v);
}

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

float getSunHeight(vec3 sunDir)
{
    return clamp(sunDir.y / 0.819, 0.0, 1.0);
}

float getDayFactor(vec3 sunDir)
{
    return clamp(sunDir.y * 10.0, 0.0, 1.0);
}

float getSunVisibility(vec3 sunDir)
{
    return smoothstep(0.08, 0.22, sunDir.y);
}

float getSunriseAmount(vec3 sunDir)
{
    return smoothstep(-0.20, 0.10, sunDir.y);
}

float getSunHaloVisibility(vec3 sunDir)
{
    return smoothstep(-0.12, 0.15, sunDir.y);
}

vec3 applyHorizonBase(vec3 viewDir)
{
    vec3 lowerColor = vec3(0.0, 0.0, 0.0);
    vec3 upperColor = vec3(0.02, 0.06, 0.16);

    float horizonMask = smoothstep(-0.01, 0.01, viewDir.y);

    return mix(lowerColor, upperColor, horizonMask);
}

vec3 getSunDiskColor(vec3 sunDir)
{
    float sunHeight = getSunHeight(sunDir);

    vec3 lowSunColor  = vec3(1.0, 0.15, 0.02);
    vec3 midSunColor  = vec3(1.0, 0.75, 0.25);
    vec3 highSunColor = vec3(1.0, 1.0, 0.9);

    vec3 color = mix(
        lowSunColor,
        midSunColor,
        smoothstep(0.0, 0.15, sunHeight)
    );

    color = mix(
        color,
        highSunColor,
        smoothstep(0.15, 0.4, sunHeight)
    );

    return color;
}

vec3 addSunDisk(vec3 currentColor, vec3 viewDir, vec3 sunDir)
{
    float cosTheta = dot(viewDir, sunDir);

    float sunDisk = smoothstep(0.99995, 0.99998, cosTheta);
    
    vec3 sunColor = getSunDiskColor(sunDir);

    float visibility = getSunVisibility(sunDir);

    vec3 directSun = sunColor * sunDisk * visibility * 15000.0;

    return currentColor + directSun;
}

vec3 getSunHaloColor(vec3 sunDir)
{
    float sunHeight = getSunHeight(sunDir);

    vec3 lowHaloColor  = vec3(1.0, 0.25, 0.05);
    vec3 midHaloColor  = vec3(1.0, 0.75, 0.3);
    vec3 highHaloColor = vec3(1.0, 1.0, 1.0);

    vec3 color = mix(
        lowHaloColor,
        midHaloColor,
        smoothstep(0.0, 0.15, sunHeight)
    );

    color = mix(
        color,
        highHaloColor,
        smoothstep(0.15, 0.4, sunHeight)
    );

    return color;
}

vec3 addSunHalo(vec3 currentColor, vec3 viewDir, vec3 sunDir)
{
    float cosTheta = dot(viewDir, sunDir);

    float mie = miePhase(cosTheta, 0.99);

    float betaM = 21e-6 * 0.1;

    float h = max(viewDir.y, 0.0) + 0.15;
    float atmosphereThickness = 1.0 / h;

    vec3 haloColor = getSunHaloColor(sunDir);

    float haloVisibility = getSunHaloVisibility(sunDir);

    vec3 halo = atmosphereThickness * betaM * mie * haloColor * haloVisibility;

    return currentColor + halo * 40000.0;
}

vec3 getSkyTint(vec3 viewDir, vec3 sunDir)
{
    float sunHeight = getSunHeight(sunDir);
    float sunriseAmount = getSunriseAmount(sunDir);

    float verticalGradient = smoothstep(0.0, 0.6, max(viewDir.y, 0.0));

    vec3 nightZenith = vec3(0.02, 0.04, 0.10);
    vec3 sunriseHorizon = vec3(1.0, 0.32, 0.45);
    vec3 sunriseZenith = vec3(0.15, 0.08, 0.25);

    vec3 sunriseSky = mix(
        sunriseHorizon,
        sunriseZenith,  
        verticalGradient
    );

    vec3 dayBlue = vec3(0.4, 0.7, 1.0);
    vec3 highSky = vec3(1.0, 1.0, 1.0);

    vec3 skyTint = mix(
        nightZenith,
        sunriseSky,
        sunriseAmount
    );

    skyTint = mix(
        skyTint,
        dayBlue,
        smoothstep(0.05, 0.25, sunHeight)
    );

    skyTint = mix(
        skyTint,
        highSky,
        smoothstep(0.25, 0.5, sunHeight)
    );

    return skyTint;
}

vec3 addSkyScattering(vec3 currentColor, vec3 viewDir, vec3 sunDir)
{
    float cosTheta = dot(viewDir, sunDir);

    float rayleigh = rayleighPhase(cosTheta);

    vec3 betaR = vec3(5.5e-6, 13.0e-6, 22.4e-6);

    float h = max(viewDir.y, 0.0) + 0.15;
    float atmosphereThickness = 1.0 / h;

    vec3 skyTint = getSkyTint(viewDir, sunDir);

    vec3 scatteredRayleigh =
        atmosphereThickness *
        betaR *
        rayleigh *
        skyTint;

    return currentColor + scatteredRayleigh * 40000.0;
}

vec3 getNightGroundAmbient(vec3 viewDir)
{
    vec3 groundColor = vec3(0.025, 0.025, 0.035);
    vec3 horizonGlow = vec3(0.08, 0.085, 0.12);

    float horizonMask = smoothstep(-0.4, 0.0, viewDir.y);

    return mix(groundColor, horizonGlow, horizonMask);
}

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec3 getStars(vec3 viewDir, vec3 sunDir)
{
    float nightAmount = 1.0 - smoothstep(-0.10, 0.05, sunDir.y);

    float aboveHorizon = smoothstep(0.02, 0.20, viewDir.y);

    vec2 uv = directionToPolarUV(viewDir);

    vec2 starScale = vec2(360.0, 120.0);

    vec2 starGrid = uv * starScale;

    vec2 cell = floor(starGrid);
    vec2 local = fract(starGrid) - 0.5;

    float randomValue = hash21(cell);

    float starExists = step(0.985, randomValue);

    float size = mix(0.06, 0.18, hash21(cell + 12.7));

    float dist = length(local);

    float starShape = 1.0 - smoothstep(size * 0.4, size, dist);

    float star = starExists * starShape * nightAmount * aboveHorizon;

    return vec3(star);
}

vec3 addPreSunriseHorizonGlow(vec3 currentColor, vec3 viewDir, vec3 sunDir) 
{
    float sunY = sunDir.y;

    // Move the apparent atmospheric horizon slightly above the geometric horizon.
    // Negative values put the terrain a little below the glow line on screen.
    const float horizonOffset = -0.025;
    float y = viewDir.y - horizonOffset;
    float skyOnly = smoothstep(-0.015, 0.025, y);

    // 0 while the sun is well below the horizon, 1 once it reaches early day.
    float sunriseProgress = smoothstep(-0.10, 0.10, sunY);

    // Fade this custom sunrise layer in before dawn and out after the day sky takes over.
    float dawnIn = smoothstep(-0.13, -0.08, sunY);
    float dayTakeover = smoothstep(0.08, 0.20, sunY);
    float sunriseAmount = dawnIn * (1.0 - dayTakeover);

    // The red veil climbs higher as the sun approaches the horizon.
    float redTop = mix(0.08, 0.55, sunriseProgress);
    float redVerticalMask = (1.0 - smoothstep(0.0, redTop, max(y, 0.0))) * skyOnly;

    vec3 deepRed = vec3(0.42, 0.055, 0.075);
    vec3 roseRed = vec3(0.95, 0.20, 0.14);
    vec3 redVeilColor = mix(deepRed, roseRed, sunriseProgress);

    // A tighter band near the horizon warms from red-orange to golden yellow.
    float goldBandHeight = mix(0.035, 0.16, sunriseProgress);
    float goldMask = (1.0 - smoothstep(0.0, goldBandHeight, abs(y))) * skyOnly;

    vec3 ember = vec3(0.75, 0.18, 0.05);
    vec3 gold = vec3(1.0, 0.72, 0.18);
    vec3 goldColor = mix(ember, gold, sunriseProgress);

    // The thin white line only appears as the sun reaches and clears the horizon.
    float whiteLineShape =
        smoothstep(-0.018, -0.003, y) *
        (1.0 - smoothstep(0.004, 0.035, y));
    float whiteLineAmount = whiteLineShape * smoothstep(0.0, 0.10, sunY);

    vec3 whiteHot = vec3(1.0, 0.96, 0.72);

    vec3 glow = vec3(0.0);
    glow += redVeilColor * redVerticalMask * mix(0.18, 0.55, sunriseProgress);
    glow += goldColor * goldMask * mix(0.20, 0.85, sunriseProgress);
    glow += whiteHot * whiteLineAmount * 1.35;

    return currentColor + glow * sunriseAmount;
}

vec3 getNightAmbient(vec3 viewDir, vec3 sunDir)
{
    float nightAmount = 1.0 - smoothstep(-0.05, 0.15, sunDir.y);

    float skyY = max(viewDir.y, 0.0);

    // 0 near horizon, 1 higher in the sky.
    float darkBlueFactor = smoothstep(0.0, 0.45, skyY);

    // 0 at mid sky, 1 near the top.
    float blackFactor = smoothstep(0.35, 1.0, skyY);

    vec3 nightHorizonColor = vec3(0.09, 0.10, 0.16);
    vec3 darkBlue     = vec3(0.015, 0.025, 0.075);
    vec3 pitchBlack   = vec3(0.0, 0.0, 0.0);

    // Merge three colors along the horizon to mid sky and mid sky to zeith.
    vec3 nightSky = mix(
        nightHorizonColor,
        darkBlue,
        darkBlueFactor
    );

    nightSky = mix(
        nightSky,
        pitchBlack,
        blackFactor
    );

    // Calculate the color for the horizon and below the grownd 
    vec3 groundColor = getNightGroundAmbient(viewDir);//vec3(0.025, 0.025, 0.035);

    // This variable controls the range around which the ground and hight sky mix
    float aboveHorizon = smoothstep(-0.01, 0.01, viewDir.y);

    vec3 nightScene = mix(groundColor, nightSky, aboveHorizon);

    return nightScene * nightAmount;
}

vec3 computeSkyNew(vec3 viewDir, vec3 sunDir)
{
    float dayFactor = getDayFactor(sunDir);

    vec3 nightColor = getNightAmbient(viewDir, sunDir);

    nightColor = addPreSunriseHorizonGlow(nightColor, viewDir, sunDir);

    nightColor += getStars(viewDir, sunDir);

    vec3 dayColor = vec3(0.0);
    dayColor = applyHorizonBase(viewDir);
    dayColor = addSkyScattering(dayColor, viewDir, sunDir);
    dayColor = addSunHalo(dayColor, viewDir, sunDir);
    dayColor = addSunDisk(dayColor, viewDir, sunDir);

    vec3 color = nightColor + dayColor * dayFactor;

    return color;
}

// Old 
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
    vec3 skyColor = computeSkyNew(viewDir, sunDirection) ;
    FragColor = vec4(skyColor, 1.0);
}
