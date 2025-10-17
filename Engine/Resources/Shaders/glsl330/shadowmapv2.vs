#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp; // default keyword
uniform mat4 matModel; // default keyword
uniform mat4 matLightVP;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec2 fragShadowTexCoord;
out float fragShadowDepth;

void main(){
    gl_Position = mvp*vec4(vertexPosition, 1.0);
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    vec4 worldSpace = matModel*vec4(vertexPosition, 1.0); // position of the model in the scene
    vec4 screenSpace = matLightVP*worldSpace; // position of the vertex in screen space. equivalent to gl_Position above but for the light
    fragShadowDepth = screenSpace.z/screenSpace.w; // .z component is depth in screen space.
    fragShadowTexCoord = (screenSpace.xy/screenSpace.w)*.5+.5; // .xy is position on the screen
}