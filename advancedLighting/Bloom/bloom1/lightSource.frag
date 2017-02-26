#version 330
layout (location = 0) out vec4 FragColor;  // 指定输出到第一个color buffer
layout (location = 1) out vec4 BrightColor;// 指定输出到第二个color buffer

// 定义输入interface block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;
}fs_in;

uniform vec3 lightColor;

//out vec4 FragColor;

void main()
{   
	FragColor = vec4(lightColor, 1.0f); // 对于模拟的光源只输出光源本身颜色
	// 根据高亮部分阈值 决定输出到高亮对应的color buffer
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.0f)
		BrightColor = vec4(FragColor.rgb, 1.0f);
}