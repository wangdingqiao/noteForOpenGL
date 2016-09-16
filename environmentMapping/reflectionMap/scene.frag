#version 330 core

in vec3 FragNormal;
in vec3 FragPos;
in vec2 TextCoord;

uniform samplerCube envText;	// 环境纹理
uniform sampler2D texture_diffuse0;
uniform sampler2D specular_diffuse0;
uniform sampler2D texture_reflection0; // 反射map
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	vec4	diffuseColor = texture(texture_diffuse0, TextCoord);
	float	relefctIntensity = texture(texture_reflection0, TextCoord).r; 
	vec4	reflectColor = vec4(0.0, 0.0, 0.0, 0.0);
	if(relefctIntensity > 0.1) // 决定是否开启reflection map
	{
		vec3 viewDir = normalize(FragPos - cameraPos); // 注意这里向量从观察者位置指出
		vec3 reflectDir = reflect(viewDir, normalize(FragNormal));
		reflectColor = texture(envText, reflectDir) * relefctIntensity;	// 使用反射向量采样环境纹理 使用强度系数控制
	}
	color = diffuseColor + reflectColor;
}