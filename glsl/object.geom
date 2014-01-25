#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 o_vert[3];
out vec3 o_norm;

void main()
{
	// Calculate triangle normal vector
	vec3 e1 = o_vert[0] - o_vert[1];
	vec3 e2 = o_vert[0] - o_vert[2];
	o_norm = normalize(cross(e1, e2));

	for (int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;	
		EmitVertex();
	}

	EndPrimitive();
}
