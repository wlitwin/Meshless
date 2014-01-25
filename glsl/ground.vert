#version 330

uniform mat4 proj;
uniform mat4 view;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec3 o_norm;
out vec3 o_vert;

void main()
{
	mat4 mvp = proj * view;

	o_norm = normal;
	o_vert = vertex;

	gl_Position = mvp * vec4(vertex, 1.0);
}
