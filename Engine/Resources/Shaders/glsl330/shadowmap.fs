#version 330

#define MAX_LIGHTS 15

// Input from vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Textures
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output
out vec4 finalColor;

// Lighting uniforms
uniform vec3 lightPos[MAX_LIGHTS];           // Position for point and spot lights
uniform vec3 lightDir[MAX_LIGHTS];           // Direction for directional and spot lights
uniform vec4 lightColor[MAX_LIGHTS];         // RGB + intensity in alpha channel
uniform int lightType[MAX_LIGHTS];           // 0=Point, 1=Spot, 2=Directional
uniform vec4 ambient;
uniform vec3 viewPos;

// Shadow uniforms
uniform bool enableShadows = true;
uniform mat4 lightVP[MAX_LIGHTS];
uniform sampler2D shadowMap[MAX_LIGHTS];

// Spotlight properties
uniform float spotInnerCutoff[MAX_LIGHTS];   // Cosine of inner cutoff angle
uniform float spotOuterCutoff[MAX_LIGHTS];   // Cosine of outer cutoff angle

// Material properties
const float shininess = 32.0;
const float specularStrength = 0.5;

// Calculate shadow with PCF (Percentage Closer Filtering)
float CalculateShadow(int lightIdx, vec3 fragPos, vec3 normal, vec3 lightDirection)
{
    if (!enableShadows) 
        return 0.0;
    
    // Transform fragment position to light space
    vec4 fragPosLightSpace = lightVP[lightIdx] * vec4(fragPos, 1.0);
    
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light frustum
    if (projCoords.z > 1.0 || 
        projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    
    // Calculate bias to prevent shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);
    
    // PCF (Percentage Closer Filtering) for softer shadows
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

// Calculate attenuation for point and spot lights
float CalculateAttenuation(float distance, float range)
{
    // Physically-based attenuation with range limit
    float attenuation = 1.0 / (distance * distance + 1.0);
    
    // Smooth falloff at range boundary
    float rangeFactor = max(1.0 - (distance / range), 0.0);
    rangeFactor = rangeFactor * rangeFactor; // Square for smoother falloff
    
    return attenuation * rangeFactor;
}

void main()
{
    // Sample texture
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    
    // Start with ambient lighting
    vec3 lighting = ambient.rgb * ambient.a;
    
    // Process each light
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        // Skip inactive lights (alpha < 0.1)
        if (lightColor[i].a < 0.01)
            continue;
        
        vec3 lightDirNorm;
        float attenuation = 1.0;
        float shadow = 0.0;
        
        if (lightType[i] == 2) // Directional Light
        {
            // Directional lights only use direction, not position
            lightDirNorm = normalize(-lightDir[i]);
            shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
        }
        else // Point or Spot Light
        {
            // Calculate direction from fragment to light
            vec3 lightVec = lightPos[i] - fragPosition;
            float dist = length(lightVec);
            lightDirNorm = normalize(lightVec);
            
            if (lightType[i] == 0) // Point Light
            {
                // Use realistic attenuation with range
                attenuation = CalculateAttenuation(dist, 100.0); // TODO: Use per-light range
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            }
            else if (lightType[i] == 1) // Spot Light
            {
                // Calculate spotlight cone attenuation
                float theta = dot(lightDirNorm, normalize(-lightDir[i]));
                float epsilon = spotInnerCutoff[i] - spotOuterCutoff[i];
                float spotIntensity = clamp((theta - spotOuterCutoff[i]) / epsilon, 0.0, 1.0);
                
                // Combine distance and cone attenuation
                attenuation = CalculateAttenuation(dist, 100.0) * spotIntensity; // TODO: Use per-light range
                shadow = CalculateShadow(i, fragPosition, normal, lightDirNorm);
            }
        }
        
        // Calculate diffuse lighting
        float NdotL = max(dot(normal, lightDirNorm), 0.0);
        vec3 diffuse = lightColor[i].rgb * NdotL * (1.0 - shadow) * attenuation;
        
        // Calculate specular lighting (Blinn-Phong)
        vec3 specular = vec3(0.0);
        if (NdotL > 0.0 && shadow < 0.99)
        {
            vec3 halfwayDir = normalize(lightDirNorm + viewDir);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
            specular = lightColor[i].rgb * spec * specularStrength * attenuation * (1.0 - shadow);
        }
        
        // Accumulate lighting
        lighting += (diffuse + specular);
    }
    
    // Apply lighting to texture color
    vec3 finalRGB = lighting * texelColor.rgb * colDiffuse.rgb;
    
    // Apply gamma correction
    finalRGB = pow(finalRGB, vec3(1.0 / 2.2));
    
    // Output final color
    finalColor = vec4(finalRGB, texelColor.a * colDiffuse.a);
}