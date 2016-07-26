#version 330 core

in vec2 TextCoord;
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular0;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

out vec4 color;


void main()
{
	color = texture(texture_diffuse0, TextCoord);
}