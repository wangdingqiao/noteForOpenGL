#version 330 core

in vec2 TextCoord;
uniform sampler2D text;


out vec4 color;


void main()
{
	vec4 textColor = texture(text, TextCoord);
	if(textColor.a < 0.1)  // 根据alpha值 丢弃片元 < 0.1表示接近完全透明
		discard;
	color = textColor;
}