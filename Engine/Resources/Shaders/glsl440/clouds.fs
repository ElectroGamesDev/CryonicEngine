#version 440

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D cloudTexture;

void main()
{
    vec4 color = texture(cloudTexture, fragTexCoord);
    finalColor = color;
}