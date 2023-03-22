#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2)  in vec2 txc;

uniform mat4 shadowMVP;

uniform mat4 mvp;

out vec2 texCoord;
out vec4 shadowCoord;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    texCoord = txc;
    shadowCoord = shadowMVP * vec4(position, 1.0);
    //shadowCoord = vec4(txc, 1, 1);
}