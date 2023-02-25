#version 330 core

in vec2 texCoord;

out vec4 color;

uniform sampler2D tex;

vec4 texColor = texture(tex, texCoord);

void main()
{
    color = texColor;
}