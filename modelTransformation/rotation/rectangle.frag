#version 330

in vec3 VertColor;
in vec2 TextCoord;

uniform sampler2D tex;

out vec4 color;


void main()
{
	color = texture(tex, vec2(TextCoord.s, 1.0 -TextCoord.t) );
}