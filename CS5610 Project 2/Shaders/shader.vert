#version 330 core

// source of shader: https://community.khronos.org/t/adding-normals-into-fragment-shader/73506

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 txc;

uniform vec3 viewPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 camPos;
out vec2 texCoord;

void main()
{
	// calculate positions
	vec4 pos = projection * view * model * vec4(position, 1.0f);
	gl_Position = pos;
	// set other values
	FragPos = vec3(view * model * vec4(position, 1.0f));
	vec3 viewDir = viewPos;
	Normal = normal;
	texCoord = txc;
	camPos = viewPos;
}