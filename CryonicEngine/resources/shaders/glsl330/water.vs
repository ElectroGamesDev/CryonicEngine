#version 330

// Standard inputs
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Matrices (assume set by Raylib wrapper, similar to shadowmap.vs)
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;  // Normal matrix (transpose inverse of model)

// Wave uniforms
uniform float time;
uniform float waveAmp1;
uniform float waveLambda1;
uniform float waveSpeed1;
uniform vec2 waveDir1;
uniform float waveAmp2;
uniform float waveLambda2;
uniform float waveSpeed2;
uniform vec2 waveDir2;

// Outputs (matching shadowmap.fs inputs)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

const float PI = 3.14159265359;
const float G = 9.8;  // Gravity

void main() {
    vec3 pos = vertexPosition;
    
    // Compute world position for waves
    vec4 worldPos4 = matModel * vec4(pos, 1.0);
    vec2 worldXZ = worldPos4.xz;
    
    // Normalize directions
    vec2 dir1 = normalize(waveDir1);
    vec2 dir2 = normalize(waveDir2);
	
	// Gerstner-like wave displacement (simplified sine for performance)
    // Wave 1
    float k1 = 2.0 * PI / waveLambda1;  // Wave number
    float c1 = sqrt(G / k1);  // Phase speed (deep water dispersion)
    float phase1 = k1 * (dot(dir1, worldXZ) - (waveSpeed1 * c1 * time));
    float wave1 = waveAmp1 * sin(phase1);
    
    // Wave 2
    float k2 = 2.0 * PI / waveLambda2;
    float c2 = sqrt(G / k2);
    float phase2 = k2 * (dot(dir2, worldXZ) - (waveSpeed2 * c2 * time));
    float wave2 = waveAmp2 * sin(phase2);
    
    // Total displacement (y)
    pos.y += wave1 + wave2;
    
    // For better normals, compute tangent space perturbation (simplified)
    // Tangent (x-direction perturbation)
    float tan1 = waveAmp1 * k1 * cos(phase1) * dir1.x;
    float tan2 = waveAmp2 * k2 * cos(phase2) * dir2.x;
    pos.x += tan1 + tan2;  // Slight horizontal displacement for Gerstner-like choppiness
    
    // Bitangent (z-direction)
    float bitan1 = waveAmp1 * k1 * cos(phase1) * dir1.y;
    float bitan2 = waveAmp2 * k2 * cos(phase2) * dir2.y;
    pos.z += bitan1 + bitan2;
    
    // Transform to world space
    vec4 worldPos = matModel * vec4(pos, 1.0);
    fragPosition = worldPos.xyz;
    
    // Pass through texture coordinates and color
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Compute normal in world space (perturbed)
    vec3 normal = vertexNormal;
    // Approximate normal from derivatives (for lighting)
    vec3 tangent = vec3(1.0, -tan1 - tan2, 0.0);  // Simplified
    vec3 bitangent = vec3(0.0, -bitan1 - bitan2, 1.0);
    normal = normalize(cross(bitangent, tangent));  // Recompute normal from TBN
    // Or use finite difference if needed, but this is approx for multi-waves
    fragNormal = normalize(matNormal * vec4(normal, 0.0)).xyz;
    
    // Final clip space position
    gl_Position = mvp * vec4(pos, 1.0);
}