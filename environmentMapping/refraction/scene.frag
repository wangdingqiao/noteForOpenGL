#version 330 core

in vec3 FragNormal;
in vec3 FragPos;

uniform samplerCube envText; // 环境纹理
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	float indexRation = 1.00 / 1.52;
	vec3 viewDir = normalize(FragPos - cameraPos); // 注意这里向量从观察者位置指出
	vec3 refractDir = refract(viewDir, normalize(FragNormal), indexRation);
	color = texture(envText, refractDir);	// 使用 折射向量 采样环境纹理
}