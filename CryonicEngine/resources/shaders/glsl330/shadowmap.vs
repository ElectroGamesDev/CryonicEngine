#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    // Calculate world space position
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    
    // Pass through texture coordinates
    fragTexCoord = vertexTexCoord;
    
    // Pass through vertex color
    fragColor = vertexColor;
    
    // Transform normal to world space
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    
    // Calculate final vertex position (clip space)
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}