#version 330

#define MAX_LIGHTS 15

uniform bool enableShadows = true;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Terrain textures
uniform sampler2D splatmap;        // Splatmap (weights for up to 4 layers)
uniform sampler2D texture0;        // Layer 0 texture
uniform sampler2D texture1;        // Layer 1 texture
uniform sampler2D texture2;        // Layer 2 texture
uniform sampler2D texture3;        // Layer 3 texture
uniform float textureScale = 10.0; // Scale for tiling terrain textures

uniform vec4 colDiffuse;

out vec4 finalColor;

// Lighting
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightDir[MAX_LIGHTS];
uniform vec4 lightColor[MAX_LIGHTS];
uniform int lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

// Shadows
uniform mat4 lightVP[MAX_LIGHTS];
uniform sampler2D shadowMap[MAX_LIGHTS];
uniform int shadowMapResolution;

// Spotlight Properties
uniform float spotInnerCutoff[MAX_LIGHTS];
uniform float spotOuterCutoff[MAX_LIGHTS];

float CalculateShadow(int lightIdx, vec3 fragPos, vec3 normal) {
    if (!enableShadows) return 0.0;
    
    vec4 fragPosLightSpace = lightVP[lightIdx] * vec4(fragPos, 1.0);
    fragPosLightSpace.xyz /= fragPosLightSpace.w;
    vec3 projCoords = fragPosLightSpace.xyz * 0.5 + 0.5;
    
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.0005 * (1.0 - dot(normal, normalize(-lightDir[lightIdx]))), 0.0001);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap[lightIdx], 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap[lightIdx], projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    // Sample splatmap to get layer weights
    vec4 splatWeights = texture(splatmap, fragTexCoord);
    
    // Sample terrain layer textures with tiling
    vec2 tiledUV = fragTexCoord * textureScale;
    vec4 layer0 = texture(texture0, tiledUV);
    vec4 layer1 = texture(texture1, tiledUV);
    vec4 layer2 = texture(texture2, tiledUV);
    vec4 layer3 = texture(texture3, tiledUV);
    
    // Blend textures based on splatmap weights
    vec4 texelColor = layer0 * splatWeights.r +
                      layer1 * splatWeights.g +
                      layer2 * splatWeights.b +
                      layer3 * splatWeights.a;
    
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    
    vec3 lighting = ambient.rgb * texelColor.rgb;
    
    for(int i = 0; i < MAX_LIGHTS; i++) {
        if(lightColor[i].a < 0.1) continue;
        
        float shadow = 0.0;
        vec3 lightDirNorm;
        float attenuation = 1.0;
        
        if (lightType[i] == 3) { // Directional Light
            lightDirNorm = normalize(-lightDir[i]);
            shadow = CalculateShadow(i, fragPosition, normal);
        }
        else {
            vec3 lightVec = lightPos[i] - fragPosition;
            float dist = length(lightVec);
            lightDirNorm = normalize(lightVec);
            
            if (lightType[i] == 0) { // Point Light
                attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));
            }
            else if (lightType[i] == 1) { // Spot Light
                float theta = dot(lightDirNorm, normalize(-lightDir[i]));
                float epsilon = spotInnerCutoff[i] - spotOuterCutoff[i];
                float intensity = clamp((theta - spotOuterCutoff[i]) / epsilon, 0.0, 1.0);
                attenuation *= intensity;
            }
        }
        
        float NdotL = max(dot(normal, lightDirNorm), 0.0);
        
        vec3 diffuse = lightColor[i].rgb * NdotL * (1.0 - shadow) * attenuation;
        
        vec3 specular = vec3(0.0);
        if(NdotL > 0.0 && shadow < 0.99) {
            vec3 reflectDir = reflect(-lightDirNorm, normal);
            specular = lightColor[i].rgb * pow(max(dot(viewDir, reflectDir), 0.0), 16.0) * attenuation;
        }
        
        lighting += (diffuse + specular) * texelColor.rgb * colDiffuse.rgb;
    }
    
    finalColor = vec4(pow(lighting, vec3(1.0/2.2)), 1.0);
}