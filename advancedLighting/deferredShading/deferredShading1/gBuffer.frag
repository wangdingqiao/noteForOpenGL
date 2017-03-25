#version 330

layout (location = 0) out vec3 gPosition;   // 指定输出到第一个color buffer
layout (location = 1) out vec3 gNormal; // 指定输出到第二个color buffer
layout (location = 2) out vec4 gAlbedoAndSpec; // 指定输出到第三个color buffer

// 定义输入interface block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;
}fs_in;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

void main()
{
	// 输出片元位置
	gPosition = fs_in.FragPos;
	// 输出片元的法向量信息
	gNormal = normalize(fs_in.FragNormal);
	// 输出片元的漫反射光颜色
	gAlbedoAndSpec.rgb = texture(texture_diffuse0, fs_in.TextCoord).rgb;
	// 输出镜面光强度 存储为alpha成分
	gAlbedoAndSpec.a = texture(texture_specular0, fs_in.TextCoord).r;
}