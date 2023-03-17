#version 330 core

in vec2 texCoord;

out vec4 color;

uniform sampler2D tex;

void main()
{
    float depthValue = texture(tex, texCoord).r;
    if (depthValue >= 0.00000000000000001)
    {
        color = vec4(vec3(depthValue), 1.0);
    }
    else
    {
        color = vec4(1, 0.2, 0.2, 1);
    }
}