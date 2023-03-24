#version 330 core

layout ( triangles ) in;
layout ( line_strip, max_vertices = 4 ) out;

void main()
{
	// vertex 1
	gl_Position = gl_in[ 0 ].gl_Position;
	EmitVertex();

	// vertex 2
	gl_Position = gl_in[ 1 ].gl_Position;
	EmitVertex();

	// vertex 3
	gl_Position = gl_in[ 2 ].gl_Position;
	EmitVertex();

	// vertex 1 - complete loop of triangles
	gl_Position = gl_in[ 0 ].gl_Position;
	EmitVertex();
}