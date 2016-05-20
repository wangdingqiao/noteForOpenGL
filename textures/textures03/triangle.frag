#version 330

in vec3 VertColor;
in vec2 TextCoord;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float mixValue;

out vec4 color;


void main()
{
	vec4 color1 = texture(tex1, TextCoord);
	vec4 color2 = texture(tex2, vec2(TextCoord.s, 1.0 - TextCoord.t));
	color = mix(color1, color2, mixValue);
}