#version 330 core

in vec2 TextCoord;
uniform sampler2D tex;

out vec4 color;


void main()
{
	color = texture(tex, TextCoord);
}