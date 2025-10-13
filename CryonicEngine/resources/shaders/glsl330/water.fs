#version 330

#define MAX_LIGHTS 15

// Inputs from VS
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;
in float fragWaveHeight;
in vec2 fragWorldXZ;

// Uniforms (standard + water-specific)
uniform sampler2D texture0;  // Fallback to white if no texture
uniform vec4 colDiffuse;     // Base diffuse (tinted)
uniform vec4 waterColor;     // Deep water tint (e.g., dark blue)
uniform float foamThreshold; // Slope threshold for foam
uniform float waterTransparency; // Overall alpha multiplier
uniform float time;          // Time uniform for procedural animations (noise/caustics/ripples)

// Output
out vec4 finalColor;

// Lighting
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightDir[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform int lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform bool enableShadows = true;
uniform mat4 lightVP[MAX_LIGHTS];
uniform sampler2D shadowMap[MAX_LIGHTS];
uniform float spotInnerCutoff[MAX_LIGHTS];
uniform float spotOuterCutoff[MAX_LIGHTS];

// Water properties
const float fresnelPower = 5.0;
const float fresnelBias = 0.1;
const float reflectionStrength = 0.8;
const float shininess = 256.0;  // High for water specular
const float specularStrength = 1.0;
const float sssStrength = 0.3;
const float causticStrength = 0.5;
const float rippleScale = 30.0;
const float rippleAmp = 0.15;

// Simple procedural sky (gradient)
vec3 GetSkyColor(vec3 viewDir) {
    float up = max(viewDir.y, 0.0);
    return mix(vec3(0.2, 0.4, 0.8), vec3(0.8, 0.9, 1.0), up);  // Horizon to zenith
}

// Simple 2D simplex noise (procedural, no texture)
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

float fbm(vec2 p, float t) {  // Fractional Brownian Motion for caustics/ripples
    float value = 0.0;
    float amp = 0.5;
    mat2 rot = mat2(cos(t*0.1), -sin(t*0.1), sin(t*0.1), cos(t*0.1));  // Slight rotation for organic feel
    for (int i = 0; i < 4; i++) {
        value += amp * noise(p);
        p = rot * p * 2.0;
        amp *= 0.5;
    }
    return value;
}

// Calculate shadow (PCF, same as original)
float CalculateShadow(int lightIdx, vec3 fragPos, vec3 normal, vec3 lightDirection) {
    if (!enableShadows) return 0.0;
    
    vec4 fragPosLightSpace = lightVP[lightIdx] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w * 0.5 + 0.5;
    
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap[lightIdx], 0);
    
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap[lightIdx], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

// Attenuation (same)
float CalculateAttenuation(float distance, float range) {
    float att = 1.0 / (distance * distance + 1.0);
    float rangeFactor = max(1.0 - (distance / range), 0.0);
    return att * rangeFactor * rangeFactor;
}

// Improved Fresnel (Schlick with bias)
float Fresnel(vec3 I, vec3 N, float power) {
    float cosTheta = max(dot(N, -I), 0.0);
    return fresnelBias + (1.0 - fresnelBias) * pow(1.0 - cosTheta, power);
}

// Approximate depth: Use wave height and view distance for "underwater" feel
float ApproximateDepth(vec3 viewDir, float waveHeight) {
    float viewDist = length(viewPos - fragPosition);
    float depth = max(0.0, -waveHeight / max(dot(viewDir, vec3(0,1,0)), 0.1)) + viewDist * 0.01;  // Simple ray-height intersect approx
    return clamp(depth, 0.0, 10.0);
}

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    if (texColor.a < 0.01) texColor = vec4(1.0);
    
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    
    // Micro-ripples: Stronger perturbation with 3D noise (more visible on lighting)
    vec2 rippleUV = fragWorldXZ * rippleScale + time * 2.0;  // Faster for ripple motion
    float rippleNoiseX = fbm(rippleUV, time) * rippleAmp;
    float rippleNoiseZ = fbm(rippleUV * 1.5 + vec2(time * 1.5), time) * rippleAmp;  // Offset for variation
    vec3 ripplePerturb = vec3(rippleNoiseX, 0.0, rippleNoiseZ);
    normal = normalize(normal + ripplePerturb * (1.0 + abs(fragWaveHeight) * 2.0));  // Amplify on waves
    
    // Optional Debug: Tint by wave height to verify displacement (remove after testing)
    // vec3 debugTint = mix(vec3(0.0), vec3(0.2, 0.8, 0.2), abs(fragWaveHeight) * 0.5);
    // texColor.rgb *= debugTint;
    
    // Base color with water tint
    vec3 baseColor = texColor.rgb * waterColor.rgb * colDiffuse.rgb;
    
    // Approximate depth for color variation and SSS
    float depth = ApproximateDepth(viewDir, fragWaveHeight);
    vec3 depthTint = waterColor.rgb * exp(-depth * 0.2);  // Deeper = darker blue/green absorption
    baseColor *= depthTint;
    
    // Ambient (tinted)
    vec3 lighting = ambient.rgb * ambient.a * baseColor;
    
    // Lights
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lightColor[i].a < 0.01) continue;
        
        vec3 lightDirNorm;
        float att = 1.0;
        float shadow = 0.0;
        
        if (lightType[i] == 2) {  // Directional
            lightDirNorm = normalize(-lightDir[i]);
            shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
        } else {
            vec3 lightVec = lightPos[i] - fragPosition;
            float dist = length(lightVec);
            lightDirNorm = normalize(lightVec);
            
            if (lightType[i] == 0) {  // Point
                att = CalculateAttenuation(dist, 100.0);
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            } else if (lightType[i] == 1) {  // Spot
                float theta = dot(lightDirNorm, normalize(-lightDir[i]));
                float epsilon = spotInnerCutoff[i] - spotOuterCutoff[i];
                float spotInt = clamp((theta - spotOuterCutoff[i]) / epsilon, 0.0, 1.0);
                att = CalculateAttenuation(dist, 100.0) * spotInt;
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            }
        }
        
        float NdotL = max(dot(normal, lightDirNorm), 0.0);
        
        // Diffuse with SSS approx (wrap around + tint)
        vec3 sssDiffuse = lightColor[i].rgb * NdotL * (1.0 - shadow) * att * 0.3;
        sssDiffuse += lightColor[i].rgb * (1.0 + NdotL) * 0.5 * sssStrength * att * (1.0 - shadow) * depthTint;  // SSS wrap
        vec3 diffuse = sssDiffuse;
        
        // Improved specular (Fresnel modulated, boosted on ripples)
        vec3 specular = vec3(0.0);
        if (NdotL > 0.0) {
            vec3 H = normalize(lightDirNorm + viewDir);
            float spec = pow(max(dot(normal, H), 0.0), shininess);
            float specFresnel = Fresnel(viewDir, normal, fresnelPower * 0.5);  // Softer Fresnel for specular
            float rippleBoost = 1.0 + length(ripplePerturb) * 2.0;  // Extra shine on ripples
            specular = lightColor[i].rgb * spec * specularStrength * att * (1.0 - shadow) * specFresnel * rippleBoost;
        }
        
        lighting += (diffuse + specular);
    }
    
    // Caustics-like shimmer: Procedural noise modulated by depth and light (stronger on waves)
    vec2 causticUV = fragWorldXZ * 0.5 + time * 0.2;  // Slower animation for caustics
    float causticNoise = fbm(causticUV, time) * 2.0 - 1.0;  // -1 to 1 range
    float caustic = max(0.0, causticNoise) * causticStrength * (1.0 - depth * 0.1);  // Fade with depth
    caustic *= (1.0 + abs(fragWaveHeight) * 1.5);  // Boost on wave crests for shimmer
    lighting += caustic * baseColor * 0.8;  // Increased multiplier for more visible spots
    
    // Foam/Whitecaps: Detect high slopes (steep waves) using normal deviation from up
    float slope = 1.0 - max(dot(normal, vec3(0,1,0)), 0.0);  // 0= flat, 1=steep
    float foam = smoothstep(foamThreshold, foamThreshold * 2.0, slope + abs(fragWaveHeight) * 0.1);  // Height boosts foam on crests
    // Add ripple noise to foam for dynamic whitecaps
    float rippleFoam = fbm(fragWorldXZ * 10.0 + time * 3.0, time) * 0.3;
    foam = max(foam, rippleFoam * (1.0 - depth * 0.2));  // Ripples add extra foam near surface
    vec3 foamColor = vec3(1.0, 1.0, 0.95) * lighting * 2.5;  // Brighter white foam
    vec3 waterLit = baseColor * lighting;
    vec3 surfaceColor = mix(waterLit, foamColor, foam);
    
    // Fresnel reflection: Mix lit water with sky reflection
    float fresnel = Fresnel(viewDir, normal, fresnelPower);
    vec3 sky = GetSkyColor(reflect(-viewDir, normal));  // Reflect view dir for sky lookup
    vec3 reflection = mix(surfaceColor, sky, fresnel * reflectionStrength);
    
    // Depth-based transparency: More transparent at shallow angles/depths
    float alpha = waterTransparency * (1.0 - fresnel * 0.5) * (1.0 - depth * 0.05);  // Opaque deeper, transparent at edges
    alpha = clamp(alpha, 0.1, 1.0);  // Min alpha to avoid full invisibility
    
    // Final color: Premultiply alpha for blending
    vec3 finalRGB = reflection;
    finalRGB = pow(finalRGB, vec3(1.0 / 2.2));  // Gamma correction
    
    // Output with alpha (for transparency below water)
    finalColor = vec4(finalRGB * alpha, alpha * texColor.a * colDiffuse.a * fragColor.a);
}