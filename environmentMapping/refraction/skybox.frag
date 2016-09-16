#version 330 core

in vec3 TextCoord;
uniform samplerCube  skybox;  // ´Ósampler2D¸ÄÎªsamplerCube

out vec4 color;


void main()
{
	color = texture(skybox, TextCoord);
}