#version 330

in vec3 VertColor;

out vec4 color;


void main()
{
	color = vec4(VertColor, 1.0);
}