#version 330

in vec2 TextCoord;
uniform sampler2D randomText;

out vec4 color;

void main()
{   
	vec3	result = texture(randomText, TextCoord).rgb;
	color	= vec4(result, 1.0f);
}