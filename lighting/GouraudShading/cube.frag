#version 330

in vec3 VertColor;
in vec2 TextCoord;

out vec4 color;

void main()
{   
	color	= vec4(VertColor , 1.0f);
}