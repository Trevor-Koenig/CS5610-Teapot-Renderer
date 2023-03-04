#version 330 core

layout (location = 0) in vec3 position;

uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 camView;
uniform mat4 projection;

out vec3 texCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    vec4 pos = camView * vec4((position), 0.0008);
    texCoord = pos.xyz;
}