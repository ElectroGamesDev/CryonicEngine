#version 330

// Standard inputs
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Matrices (assume set by Raylib wrapper)
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;  // Normal matrix

// Wave uniforms (4 octaves)
uniform float time;
uniform float waveAmp1, waveLambda1, waveSpeed1;
uniform vec2 waveDir1;
uniform float waveAmp2, waveLambda2, waveSpeed2;
uniform vec2 waveDir2;
uniform float waveAmp3, waveLambda3, waveSpeed3;
uniform vec2 waveDir3;
uniform float waveAmp4, waveLambda4, waveSpeed4;
uniform vec2 waveDir4;
uniform float steepness;  // Global steepness for choppiness (0-1)

// Outputs
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out float fragWaveHeight;  // For FS depth/foam
out vec2 fragWorldXZ;      // For FS noise/caustics

const float PI = 3.14159265359;
const float G = 9.8;  // Gravity

// Gerstner wave function for one octave (improved for smoother motion)
vec3 GerstnerWave(vec2 dir, float amp, float lambda, float speed, float time, vec2 worldXZ, float globalSteepness) {
    vec2 d = normalize(dir);
    float k = 2.0 * PI / lambda;  // Wave number
    float omega = speed * k;      // Angular frequency (direct use of speed for control)
    float phase = k * dot(d, worldXZ) - omega * time;
    float sinPhase = sin(phase);
    float cosPhase = cos(phase);
    
    float q = globalSteepness / k;  // Steepness per wave number (avoids over-clamp)
    q = min(q, 1.0 / k);  // Better clamping for small waves
    
    // Vertical displacement
    float y = amp * sinPhase;
    
    // Horizontal displacements (choppy)
    float x = q * amp * cosPhase * d.x;
    float z = q * amp * cosPhase * d.y;
    
    return vec3(x, y, z);
}

// Partial derivatives for normal (improved signs for correct slope)
vec3 GerstnerDeriv(vec2 dir, float amp, float lambda, float speed, float time, vec2 worldXZ, float globalSteepness) {
    vec2 d = normalize(dir);
    float k = 2.0 * PI / lambda;
    float omega = speed * k;
    float phase = k * dot(d, worldXZ) - omega * time;
    float cosPhase = cos(phase);
    float sinPhase = sin(phase);  // For dy
    
    float q = globalSteepness / k;
    q = min(q, 1.0 / k);
    
    // Derivatives: partial_x = -q * amp * k * cos(phase) * d.x, etc.
    // For normal: dz/dx ≈ -partial height / partial x (negative for upward normal)
    float dx = q * amp * k * cosPhase * d.x;  // Flipped sign for correct tangent slope
    float dz = q * amp * k * cosPhase * d.y;
    float dy = amp * k * sinPhase;  // Vertical derivative for TBN
    
    return vec3(dx, dy, dz);
}

void main() {
    vec3 pos = vertexPosition;
    
    // Compute world position for waves
    vec4 worldPos4 = matModel * vec4(pos, 1.0);
    vec2 worldXZ = worldPos4.xz;
    fragWorldXZ = worldXZ;
    
    // Sum Gerstner displacements for 4 octaves
    vec3 disp1 = GerstnerWave(waveDir1, waveAmp1, waveLambda1, waveSpeed1, time, worldXZ, steepness);
    vec3 disp2 = GerstnerWave(waveDir2, waveAmp2, waveLambda2, waveSpeed2, time, worldXZ, steepness);
    vec3 disp3 = GerstnerWave(waveDir3, waveAmp3, waveLambda3, waveSpeed3, time, worldXZ, steepness);
    vec3 disp4 = GerstnerWave(waveDir4, waveAmp4, waveLambda4, waveSpeed4, time, worldXZ, steepness);
    
    vec3 totalDisp = disp1 + disp2 + disp3 + disp4;
    pos += totalDisp;
    fragWaveHeight = totalDisp.y;  // Pass height for FS
    
    // Compute perturbed normal using summed derivatives (fixed cross order for upward normals)
    vec3 deriv1 = GerstnerDeriv(waveDir1, waveAmp1, waveLambda1, waveSpeed1, time, worldXZ, steepness);
    vec3 deriv2 = GerstnerDeriv(waveDir2, waveAmp2, waveLambda2, waveSpeed2, time, worldXZ, steepness);
    vec3 deriv3 = GerstnerDeriv(waveDir3, waveAmp3, waveLambda3, waveSpeed3, time, worldXZ, steepness);
    vec3 deriv4 = GerstnerDeriv(waveDir4, waveAmp4, waveLambda4, waveSpeed4, time, worldXZ, steepness);
    
    vec3 totalDeriv = deriv1 + deriv2 + deriv3 + deriv4;
    
    // Reconstruct normal: cross(tangent, bitangent) for right-hand rule (upward)
    vec3 tangent = vec3(1.0, -totalDeriv.x, 0.0);  // Negative for correct slope direction
    vec3 bitangent = vec3(0.0, -totalDeriv.z, 1.0);
    vec3 normal = normalize(cross(tangent, bitangent));  // Swapped order from previous for correct orientation
    
    // Fallback to original normal if derivs are near-zero (flat areas)
    if (length(totalDeriv.xy) < 0.01) {
        normal = vertexNormal;
    }
    
    // Transform to world space
    vec4 worldPos = matModel * vec4(pos, 1.0);
    fragPosition = worldPos.xyz;
    
    // Pass through
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize((matNormal * vec4(normal, 0.0)).xyz);
    
    // Final position (displaced before MVP)
    gl_Position = mvp * vec4(pos, 1.0);
}