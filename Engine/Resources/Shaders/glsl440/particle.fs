#version 440
in vec2 fragUV;
in vec4 fragColor;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform bool lit;
uniform vec3 lightDir;
uniform vec4 lightColor;
out vec4 finalColor;
void main() {
vec4 texCol = texture(texture0, fragUV);
if (texCol.a < 0.01) discard;
finalColor = texCol * fragColor;
if (lit) {
float diff = max(dot(normalize(fragNormal), normalize(lightDir)), 0.0);
finalColor.rgb *= diff * lightColor.rgb * lightColor.a + 0.2;
}
}