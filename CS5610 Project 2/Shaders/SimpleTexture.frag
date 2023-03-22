#version 330 core

in vec2 texCoord;
in vec4 shadowCoord;

uniform sampler2DShadow shadowTex;
//uniform sampler2D shadowTex;

out vec4 color;



void main()
{
    color = vec4(0.75);
    color *= textureProj(shadowTex, shadowCoord);
}