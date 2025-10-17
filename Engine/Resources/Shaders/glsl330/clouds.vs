#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 fragTexCoord;

void main() {
    gl_Position = vec4(vertexPosition, 1.0);
    fragTexCoord = vertexTexCoord;
}