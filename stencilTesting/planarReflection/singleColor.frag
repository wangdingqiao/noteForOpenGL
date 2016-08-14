#version 330 core

out vec4 color;
in vec2 TextCoord;
uniform sampler2D text;

void main()
{
	color = vec4(0.1, 0.0, 0.0, 0.6);
}