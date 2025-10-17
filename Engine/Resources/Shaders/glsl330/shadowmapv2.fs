#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

in vec2 fragShadowTexCoord;
in float fragShadowDepth;

uniform sampler2D texture0; // diffuse texture keyword, DrawMesh uses this
uniform sampler2D texture_shadowmap; // custom uniform

out vec4 finalColor;

void main(){
    finalColor = fragColor * texture( texture0, fragTexCoord ); // get the colour that the fragment should be
    float shadowDepth = texture(texture_shadowmap, fragShadowTexCoord).r; // get the depth from the shadowmap at the transformed shadow position
    if(fragShadowDepth + 0.001 > shadowDepth){ // test fragShadowDepth against shadow sample
        // fragment is in shadow, do whatever you want to the colour 
        finalColor = vec4(finalColor.rgb*.2,finalColor.a);
    }
}