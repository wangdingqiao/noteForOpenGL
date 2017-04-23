#version 330

in vec2 TextCoord;
uniform sampler2D cubeText;

out vec4 color;

void main()
{   
	vec3	result = texture(cubeText, TextCoord).rgb;
	color	= vec4(result, 1.0f);
}