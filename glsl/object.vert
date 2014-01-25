#version 330

uniform mat4 proj;
uniform mat4 view;

layout(location = 0) in vec3 vertex;

out vec3 o_vert;

void main()
{
	mat4 mvp = proj * view;

	o_vert = vertex;

	gl_Position = mvp * vec4(vertex, 1.0);
}
