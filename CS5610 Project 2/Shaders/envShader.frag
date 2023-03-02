#version 330 core

in vec3 texCoord;

out vec4 color;

uniform samplerCube tex;

vec4 texColor = texture(tex, texCoord);

void main()
{
    color = texColor;
}