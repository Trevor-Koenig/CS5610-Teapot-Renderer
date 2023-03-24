#version 330 core


layout (location = 0) in vec3 position;

uniform mat4 mvp;

void main()
{
	// calculate positions
	vec4 pos = mvp * vec4(position, 1.0f);
	gl_Position = pos;
}