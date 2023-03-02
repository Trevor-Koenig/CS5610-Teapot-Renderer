#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2)  in vec2 txc;

uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 camView;
uniform mat4 projection;

out vec3 texCoord;

void main()
{
    gl_Position = projection * view * vec4(position, 1.0);
    vec4 pos = inverse(projection * camView) * vec4((position), 0.001);
    texCoord = vec3(pos.x, pos.y, pos.z);
}