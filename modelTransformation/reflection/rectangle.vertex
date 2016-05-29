#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 textCoord;

out vec3 VertColor;
out vec2 TextCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	VertColor = color;
	TextCoord = textCoord;
}