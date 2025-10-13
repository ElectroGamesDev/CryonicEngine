#version 330

#define MAX_LIGHTS 15

// Input from vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

// Textures
uniform sampler2D texture0;  // Optional: normal map or noise for ripples
uniform vec4 colDiffuse;    // Base material color (overridden by waterColor)

// Output
out vec4 finalColor;

// Water-specific uniforms
uniform vec4 waterColor;    // Deep water tint (e.g., dark blue)
uniform float fresnelPower = 5.0;  // Fresnel exponent
uniform float reflectionStrength = 0.6;  // How much to reflect sky

// Lighting uniforms (same as your lit shader)
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightDir[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform int lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

// Shadow uniforms (same)
uniform bool enableShadows = true;
uniform mat4 lightVP[MAX_LIGHTS];
uniform sampler2D shadowMap[MAX_LIGHTS];

// Spotlight properties (same)
uniform float spotInnerCutoff[MAX_LIGHTS];
uniform float spotOuterCutoff[MAX_LIGHTS];

// Water material properties (overridden for water)
const float shininess = 128.0;  // Higher for specular highlight
const float specularStrength = 0.8;  // Stronger specular for water

// Simple sky color (extend with cubemap sampling: uniform samplerCube skyCubemap;)
vec3 skyColor = vec3(0.5, 0.7, 1.0);  // Default sky blue

// Calculate shadow (same as your shader)
float CalculateShadow(int lightIdx, vec3 fragPos, vec3 normal, vec3 lightDirection)
{
    if (!enableShadows) 
        return 0.0;
    
    vec4 fragPosLightSpace = lightVP[lightIdx] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.z > 1.0 || 
        projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap[lightIdx], 0);
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap[lightIdx], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    return shadow / 9.0;
}

// Calculate attenuation (same)
float CalculateAttenuation(float distance, float range)
{
    float attenuation = 1.0 / (distance * distance + 1.0);
    float rangeFactor = max(1.0 - (distance / range), 0.0);
    rangeFactor = rangeFactor * rangeFactor;
    return attenuation * rangeFactor;
}

// Fresnel approximation (Schlick)
float Fresnel(vec3 viewDir, vec3 normal, float power)
{
    float cosTheta = max(dot(normal, viewDir), 0.0);
    return pow(1.0 - cosTheta, power);
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);  // If no texture, set to vec4(1.0)
    if (texelColor.a < 0.01) texelColor = vec4(1.0);  // Fallback
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    
    // Apply water tint to base color
    vec3 baseColor = texelColor.rgb * waterColor.rgb * colDiffuse.rgb;
    
    // Start with ambient (tinted blue)
    vec3 lighting = ambient.rgb * ambient.a * baseColor;
    
    // Process lights (same as your shader, but reduced diffuse for water)
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lightColor[i].a < 0.01) continue;
        
        vec3 lightDirNorm;
        float attenuation = 1.0;
        float shadow = 0.0;
        
        if (lightType[i] == 2)  // Directional
        {
            lightDirNorm = normalize(-lightDir[i]);
            shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
        }
        else  // Point or Spot
        {
            vec3 lightVec = lightPos[i] - fragPosition;
            float dist = length(lightVec);
            lightDirNorm = normalize(lightVec);
            
            if (lightType[i] == 0)  // Point
            {
                attenuation = CalculateAttenuation(dist, 100.0);
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            }
            else if (lightType[i] == 1)  // Spot
            {
                float theta = dot(lightDirNorm, normalize(-lightDir[i]));
                float epsilon = spotInnerCutoff[i] - spotOuterCutoff[i];
                float spotIntensity = clamp((theta - spotOuterCutoff[i]) / epsilon, 0.0, 1.0);
                attenuation = CalculateAttenuation(dist, 100.0) * spotIntensity;
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            }
        }
        
        // Diffuse (reduced for water translucency)
        float NdotL = max(dot(normal, lightDirNorm), 0.0);
        vec3 diffuse = lightColor[i].rgb * NdotL * (1.0 - shadow) * attenuation * 0.2;  // 0.2 factor
        
        // Specular (stronger)
        vec3 specular = vec3(0.0);
        if (NdotL > 0.0 && shadow < 0.99)
        {
            vec3 halfwayDir = normalize(lightDirNorm + viewDir);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
            specular = lightColor[i].rgb * spec * specularStrength * attenuation * (1.0 - shadow);
        }
        
        lighting += (diffuse + specular);
    }
    
    // Fresnel reflection (simple sky)
    float fresnel = Fresnel(-viewDir, normal, fresnelPower);  // -viewDir for incident
    vec3 reflection = mix(baseColor * lighting, skyColor, fresnel * reflectionStrength);
    
    // Combine
    vec3 finalRGB = reflection * baseColor;  // Multiply by base for tint
    
    // Gamma correction
    finalRGB = pow(finalRGB, vec3(1.0 / 2.2));
    
    // Output
    finalColor = vec4(finalRGB, texelColor.a * colDiffuse.a * fragColor.a);
}