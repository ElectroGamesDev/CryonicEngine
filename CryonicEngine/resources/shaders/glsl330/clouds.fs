#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform float time;
uniform float coverage;
uniform float density;
uniform vec2 wind;
uniform float cloudHeight;
uniform float cloudThickness;
uniform int raymarchSteps;
uniform float absorption;
uniform float scattering;
uniform float phaseG;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform float sunIntensity;
uniform float ambientLight;
uniform bool highQuality;

// Constants
const float PI = 3.14159265359;
const float EARTH_RADIUS = 6360e3;  // Approximate
const float ATMOSPHERE_HEIGHT = 100e3;

// 3D Simplex Noise (complete standard Ashima implementation)
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}
vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx
  //   x1 = x0 - i1  + 1.0 * C.xxx
  //   x2 = x0 - i2  + 2.0 * C.xxx
  //   x3 = x0 - 1.0 + 3.0 * C.xxx
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

  // Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

  // Gradients: 7x7 points over a square, mapped onto an octahedron.
  vec4 j = p - 49.0 * floor(p / 49.0 * (0.96 / 49.0));
  vec4 x_ = floor(j / 7.0);
  vec4 y_ = floor(j - 7.0 * x_ );

  vec4 x = (x_ * 2.0 + 0.5) / 7.0 - 0.5;
  vec4 y = (y_ * 2.0 + 0.5) / 7.0 - 0.5;

  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyw ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzzw ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  vec4 m = max(0.6 - vec4(dot(x,x), dot(x,y), dot(y,y), dot(y,y) ), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

// FBM for multi-octave noise
float fbm(vec3 p, float t) {
    float value = 0.0;
    float amp = 0.5;
    float freq = 1.0;
    mat3 rot = mat3(cos(t*0.1), -sin(t*0.1), 0.0,
                    sin(t*0.1),  cos(t*0.1), 0.0,
                    0.0,         0.0,         1.0);
    for (int i = 0; i < 6; i++) {
        value += amp * snoise(p * freq);
        p = rot * p * 2.0;
        amp *= 0.5;
        freq *= 2.0;
    }
    return value;
}

// Forward declaration for hash3 so worley() can call it before its definition
vec3 hash3(vec3 p);

// Worley noise for cellular patterns (clouds have cellular patterns)
float worley(vec3 p) {
    vec3 pi = floor(p);
    float mindist = 1e10;
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec3 offset = vec3(float(x), float(y), float(z));
                vec3 point = (hash3(pi + offset) - 0.5) * 2.0;  // Uses fixed hash3
                float dist = length(point + offset - (p - pi));
                mindist = min(mindist, dist);
            }
        }
    }
    return mindist;
}

// Fixed hash3 with component-wise operations (ensured it's a complete function matching the call)
vec3 hash3(vec3 p) {
    p = vec3( dot(p, vec3(127.1, 311.7, 74.7)),
              dot(p, vec3(269.5, 183.3, 246.1)),
              dot(p, vec3(113.5, 271.9, 124.6)) );
    p = mod289(p);
    p = mod289(p * vec3(3.14159265, 2.718281828, 1.6180339887));
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    return p * 2.0 - 1.0;  // Output vec3 in [-1, 1] range
}

// Combined density: FBM for shape + Worley for detail, thresholded by coverage
float sampleDensity(vec3 pos, float heightFrac) {
    vec3 windOffset = vec3(wind * time * 0.01, 0.0);
    pos += windOffset;
    vec3 p = pos / cloudHeight * 10.0;
    float noise = fbm(p, time * 0.1);
    float detail = worley(p * 2.0 + vec3(time * 0.05));
    float densityVal = (noise * 0.5 + 0.5) * (1.0 - detail * 0.5);
    float hFalloff = 1.0 - (heightFrac * 2.0 - 1.0) * (heightFrac * 2.0 - 1.0);
    densityVal *= hFalloff * 0.5 + 0.5;
    float threshold = 1.0 - coverage;
    densityVal = max(0.0, (densityVal - threshold) / (1.0 - threshold)) * density;
    return clamp(densityVal, 0.0, 1.0);
}

// Henyey-Greenstein phase function
float heneyGreenstein(vec3 dir, vec3 lightDir, float g) {
    float mu = dot(dir, lightDir);
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * mu, 1.5));
}

// Beer-Lambert transmittance
float transmittance(float dist, float density) {
    return exp(-dist * density * absorption);
}

// Ray-sphere intersection for atmosphere (optional, but for cloud slab use AABB)
vec2 rayBoxIntersect(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax) {
    vec3 invRd = 1.0 / rd;
    vec3 tmin = (boxMin - ro) * invRd;
    vec3 tmax = (boxMax - ro) * invRd;
    vec3 realMin = min(tmin, tmax);
    vec3 realMax = max(tmin, tmax);
    float tNear = max(max(realMin.x, realMin.y), realMin.z);
    float tFar = min(min(realMax.x, realMax.y), realMax.z);
    return vec2(tNear, tFar);
}

// Reconstruct world position from NDC (for ray start/end, but since full sky, start from cam)
vec3 getRayDir(vec2 uv) {
    vec4 clip = vec4(uv * 2.0 - 1.0, -1.0, 1.0);
    vec4 view = invViewProj * clip;
    return normalize(view.xyz / view.w);
}

void main() {
    float CLOUD_MIN_HEIGHT = cloudHeight - cloudThickness / 2.0;
    float CLOUD_MAX_HEIGHT = cloudHeight + cloudThickness / 2.0;

    vec2 uv = fragTexCoord;
    vec3 rayDir = getRayDir(uv);
    vec3 rayOrigin = cameraPos;

    vec3 slabMin = vec3(-1e6, CLOUD_MIN_HEIGHT, -1e6);
    vec3 slabMax = vec3(1e6, CLOUD_MAX_HEIGHT, 1e6);
    vec2 tHit = rayBoxIntersect(rayOrigin, rayDir, slabMin, slabMax);

    if (tHit.x > tHit.y || tHit.y < 0.0) {
        finalColor = vec4(0.0);
        return;
    }

    float tStart = max(tHit.x, 0.0);
    float tEnd = tHit.y;
    float stepSize = (tEnd - tStart) / float(raymarchSteps);
    if (highQuality) stepSize /= 1.5;

    vec3 lightDir = normalize(-sunDir);
    float cosTheta = dot(rayDir, lightDir);

    vec3 transmittance = vec3(1.0);
    vec3 scatteredLight = vec3(0.0);
    float totalDensity = 0.0;

    for (int i = 0; i < raymarchSteps; i++) {
        float t = tStart + stepSize * (float(i) + 0.5);
        vec3 pos = rayOrigin + rayDir * t;
        float heightFrac = (pos.y - CLOUD_MIN_HEIGHT) / cloudThickness;
        float dens = sampleDensity(pos, heightFrac);
        if (dens < 0.001) continue;
        transmittance *= exp(-stepSize * dens * absorption);
        float phase = heneyGreenstein(rayDir, lightDir, phaseG);
        vec3 lightContrib = sunColor * sunIntensity * phase * max(0.0, cosTheta) * dens * stepSize * scattering;
        vec3 totalLight = lightContrib + ambientLight * vec3(0.4, 0.6, 1.0);
        scatteredLight += dens * totalLight * transmittance;
        totalDensity += dens * stepSize;
        if (totalDensity > 2.0) break;
    }

    vec3 cloudColor = scatteredLight + vec3(0.8) * ambientLight * (1.0 - transmittance);
    cloudColor = pow(cloudColor, vec3(1.0 / 2.2));
    finalColor = vec4(cloudColor, 1.0);
}
