#version 330

in vec2 fragTexCoord;

out vec4 finalColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform float time;
uniform float coverage;        // 0..1
uniform float density;         // overall opacity scale
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

// Tunables (change these to get different cloud sizes/detail)
const float NOISE_SCALE = 1.0 / 400.0;  // smaller -> bigger cloud features (world units -> noise)
const float WORLEY_SCALE = 2.0;        // worley detail frequency relative to base
const float DETAIL_BLEND = 0.35;       // how much worley modulates shape
const float VERTICAL_SHARPNESS = 1.1;  // higher -> thinner vertical band

// --- DEBUG toggles ----------------------------------------------------------
// #define DEBUG_SHOW_RAYDIR   0   // renders normalized ray direction as color
// #define DEBUG_SHOW_DENSITY  0   // renders single-sample density at slab midpoint
// ---------------------------------------------------------------------------

const float PI = 3.14159265359;
const float EPS = 1e-6;

// Helper functions
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v) {
  const vec2 C = vec2(1.0/6.0, 1.0/3.0);
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);
  vec3 i = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx);
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy;
  vec3 x3 = x0 - D.yyy;
  i = mod289(i);
  vec4 p = permute( permute( permute(
               i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
             + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
             + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
  vec4 j = p - 49.0 * floor(p / 49.0);
  vec4 x_ = floor(j / 7.0);
  vec4 y_ = floor(j - 7.0 * x_);
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
  p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;
  vec4 m = max(0.6 - vec4(dot(x,x), dot(x,y), dot(y,y), dot(y,y) ), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
}

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

vec3 hash3(vec3 p) {
    float h1 = fract(sin(dot(p, vec3(127.1, 311.7,  74.7))) * 43758.5453123);
    float h2 = fract(sin(dot(p, vec3(269.5, 183.3, 246.1))) * 43758.5453123);
    float h3 = fract(sin(dot(p, vec3(113.5, 271.9, 124.6))) * 43758.5453123);
    return vec3(h1, h2, h3);  // Remove *2.0 - 1.0 here
}

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float worley(vec3 p) {
    vec3 pi = floor(p);
    float mindist = 1e10;
    for (int z = -1; z <= 1; ++z) {
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                vec3 off = vec3(float(x), float(y), float(z));
                vec3 point = hash3(pi + off);            // now continuous
                float dist = length(point + off - (p - pi));
                mindist = min(mindist, dist);
            }
        }
    }
    // Normalize and smooth the distance into [0,1]
    // max possible distance in a cell ~ sqrt(3) ~ 1.732
    float v = clamp(mindist / 1.732, 0.0, 1.0);
    // invert/smooth so returned value is 1.0 in 'open' areas, 0.0 near feature centers
    return smoothstep(0.0, 0.8, v);
}

// Improved sampleDensity: large-scale FBM shape + worley detail + vertical falloff
float sampleDensity(vec3 pos, float heightFrac) {
    // Wind motion
    vec3 windOffset = vec3(wind * time * 0.005, 0.0);
    pos += windOffset;

    // Coarser base noise (large cloud blobs)
    //vec3 p = pos / cloudHeight * 1.5; // smaller frequency = larger blobs
	vec3 p = pos * NOISE_SCALE; 
    float base = fbm(p, time * 0.05); 
    base = smoothstep(0.35, 0.7, base); // threshold to break up coverage

    // Add fine detail noise only inside dense areas
    //float detail = worley(p * 3.0 + vec3(time * 0.02));
	float detail = worley(p * WORLEY_SCALE + vec3(time * 0.02));
    detail = mix(1.0, detail, 0.4); // don’t overpower shape

    // Combine
    float densityVal = base * detail;

    // Vertical falloff (fade top/bottom)
    float h = clamp(heightFrac, 0.0, 1.0);
    float hFalloff = smoothstep(0.0, 0.15, h) * (1.0 - smoothstep(0.8, 1.0, h));
    densityVal *= hFalloff;

    // Apply coverage & density multiplier
    float threshold = 1.0 - coverage;
    densityVal = max(0.0, (densityVal - threshold) / (1.0 - threshold)) * density;

    return clamp(densityVal, 0.0, 1.0);
}

// Henyey-Greenstein phase function (safe)
float henyeyGreenstein(vec3 dir, vec3 lightDir, float g) {
    float mu = dot(dir, lightDir);
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(max(1e-4, 1.0 + g2 - 2.0 * g * mu), 1.5));
}

vec2 rayBoxIntersect(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax) {
    vec3 invRd = vec3( (abs(rd.x) < EPS) ? 1e30 : 1.0/rd.x,
                       (abs(rd.y) < EPS) ? 1e30 : 1.0/rd.y,
                       (abs(rd.z) < EPS) ? 1e30 : 1.0/rd.z );
    vec3 tmin = (boxMin - ro) * invRd;
    vec3 tmax = (boxMax - ro) * invRd;
    vec3 realMin = min(tmin, tmax);
    vec3 realMax = max(tmin, tmax);
    float tNear = max(max(realMin.x, realMin.y), realMin.z);
    float tFar  = min(min(realMax.x, realMax.y), realMax.z);
    return vec2(tNear, tFar);
}

vec3 getRayDir(vec2 uv) {
    vec2 ndc = vec2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);
    vec4 nearPoint = invViewProj * vec4(ndc, -1.0, 1.0);
    vec4 farPoint  = invViewProj * vec4(ndc,  1.0, 1.0);
    nearPoint /= nearPoint.w;
    farPoint  /= farPoint.w;
    return normalize(farPoint.xyz - nearPoint.xyz);
}

void main() {
    vec2 uv = fragTexCoord;
    vec3 rayDir = getRayDir(uv);

    // cloud slab bounds in world units
    float CLOUD_MIN_HEIGHT = cloudHeight - cloudThickness * 0.5;
    float CLOUD_MAX_HEIGHT = cloudHeight + cloudThickness * 0.5;
    vec3 rayOrigin = cameraPos;
    vec3 slabMin = vec3(-1e6, CLOUD_MIN_HEIGHT, -1e6);
    vec3 slabMax = vec3( 1e6, CLOUD_MAX_HEIGHT,  1e6);

    vec2 tHit = rayBoxIntersect(rayOrigin, rayDir, slabMin, slabMax);
    if (tHit.x < 0.0 && tHit.y > 0.0) tHit.x = 0.0;
    if (tHit.x > tHit.y || tHit.y < 0.0) { finalColor = vec4(0.0); return; }

    int steps = max(1, raymarchSteps);
    float tStart = max(tHit.x, 0.0);
    float tEnd = tHit.y;
    float totalLen = max(1e-6, tEnd - tStart);
    float stepSize = totalLen / float(steps);
    if (highQuality) stepSize /= 1.5;

    // lighting prep
    vec3 lightDir = normalize(-sunDir);
    vec3 sunCol = sunColor * sunIntensity;

    // integration
    float accumDensity = 0.0;
    float transmittance = 1.0;    // scalar transmittance
    vec3 scatteredLight = vec3(0.0);

    vec2 pix = floor(gl_FragCoord.xy);
    float pixelJitter = hash12(pix);
    for (int i = 0; i < steps; ++i) {
        float t = tStart + (float(i) + pixelJitter) * stepSize;
        vec3 pos = rayOrigin + rayDir * t;
        float heightFrac = (pos.y - CLOUD_MIN_HEIGHT) / cloudThickness;

        float dens = clamp(sampleDensity(pos, heightFrac), 0.0, 1.0);

        if (dens > 1e-4) {
            // attenuation for this segment (scalar)
            float atten = exp(-stepSize * dens * absorption);
            // single-scatter: compute phase once
            float phase = henyeyGreenstein(rayDir, lightDir, phaseG);
            float cosTheta = max(0.0, dot(rayDir, lightDir));
            vec3 lightContrib = sunCol * (phase * cosTheta) * (dens * stepSize) * scattering;

            // ambient contribution (tinted slightly blue)
            vec3 ambient = ambientLight * vec3(0.4, 0.6, 1.0);
            vec3 totalLight = lightContrib + ambient;

            // accumulate: multiply by current transmittance (light that reaches camera)
            scatteredLight += totalLight * transmittance;

            // update scalar transmittance after accumulating light (Beer-Lambert)
            transmittance *= atten;

            accumDensity += dens * stepSize;

            if (accumDensity > 4.0) { accumDensity = 4.0; break; }
        }
    }

    // compose final color
    vec3 raw = scatteredLight + vec3(0.8) * ambientLight * (1.0 - transmittance);
    raw = min(raw, vec3(100.0));
    vec3 tone = raw / (raw + vec3(1.0));
    tone = pow(tone, vec3(1.0 / 2.2));
	// Dither to break up subtle banding
	tone += (fract(sin(dot(uv, vec2(12.9898,78.233))) * 43758.5453) - 0.5) / 255.0;


    // more physical alpha from integrated density
    float cloudAlpha = 1.0 - exp(-accumDensity * 0.6);
    finalColor = vec4(tone, clamp(cloudAlpha, 0.0, 1.0));
}
