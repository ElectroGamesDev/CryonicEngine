#version 440

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexCoord;

out vec2 fragTexCoord;

void main()
{
    // Pass the texture coordinate to the fragment shader
    fragTexCoord = vertexTexCoord;

    gl_Position = vec4(vertexPosition, 1.0);
}
