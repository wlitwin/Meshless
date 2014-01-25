#version 330

out vec4 color;

vec3 L = vec3(0, 10, 0);

in vec3 o_norm;
in vec3 o_vert;

void main()
{
	vec3 N = normalize(o_norm);

	vec3 atten = vec3(dot(N, L) / length(o_vert - L));

	if (atten.x < 0.05) atten = vec3(0.05);

	float grid_size = 2.5;
	if (mod(o_vert.x, grid_size) < 0.15 || 
		mod(o_vert.z, grid_size) < 0.15)
	{
		atten -= vec3(0.4, 0.4, 0.4);
	}


	color = vec4(atten, 1.0);
}
