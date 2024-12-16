#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord; 
layout (location = 2) in vec3 aNormal;


out vec2 TexCoord0; 
out vec3 Normal0; 
out vec3 Position0; 

uniform mat4 mwvp;

void main() { 

	// Pass texture coordinates to the fragment shader
    TexCoord0 = aTexCoord; 
    Normal0 = aNormal;
    Position0 = aPos; 

    gl_Position = mwvp * vec4(aPos, 1.0);
} 




