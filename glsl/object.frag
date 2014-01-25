#version 330

in vec3 o_vert;
in vec3 o_norm;

vec3 L = vec3(0, 10, 0);


out vec4 color;

void main()
{
	vec3 N = normalize(o_norm);
	float atten = dot(N, L) / length(o_vert - L);

	if (atten < 0.05) atten = 0.05;

	color = vec4(vec3(1)*atten, 1.0);
}
