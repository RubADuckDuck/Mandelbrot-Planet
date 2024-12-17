#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord; 
layout (location = 2) in vec3 aNormal;


out vec2 TexCoord0; 
out vec3 Normal0; 
out vec3 Position0; 

uniform mat4 mwvp;
uniform mat4 modelMatrix;   // Model matrix (to transform positions)
uniform mat3 normalMatrix;  // Normal matrix (to transform normals)

void main() { 
    // Transform position into world space
    Position0 = vec3(modelMatrix * vec4(aPos, 1.0));

    // Transform the normal vector
    Normal0 = normalize(normalMatrix * aNormal);

    // Pass texture coordinates to the fragment shader
    TexCoord0 = aTexCoord; 

    gl_Position = mwvp * vec4(aPos, 1.0);
} 