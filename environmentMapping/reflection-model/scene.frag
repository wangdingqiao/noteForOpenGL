#version 330 core

in vec3 FragNormal;
in vec3 FragPos;

uniform samplerCube envText;	// 环境纹理
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	vec3 viewDir = normalize(FragPos - cameraPos); // 注意这里向量从观察者位置指出
	vec3 reflectDir = reflect(viewDir, normalize(FragNormal));
	color = texture(envText, reflectDir);	// 使用反射向量采样环境纹理
}