#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
uniform mat4 mvp;
uniform mat4 invTransView;
out vec4 vcolor;

void main()
{
	gl_Position = mvp * vec4(pos, 1);
	vcolor = invTransView * vec4(normalize(normal), 1);
}