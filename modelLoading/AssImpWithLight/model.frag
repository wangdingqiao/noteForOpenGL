#version 330 core

in vec3 FragPos;
in vec2 TextCoord;
in vec3 FragNormal;

// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;	// 衰减常数
	float linear;   // 衰减一次系数
	float quadratic; // 衰减二次系数
};

uniform LightAttr light;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular0;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

out vec4 color;

void main()
{
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse0, TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, TextCoord));

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - FragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 64.0f);
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, TextCoord));

	// 计算衰减因子
	float distance = length(light.position - FragPos); // 在世界坐标系中计算距离
	float attenuation = 1.0f / (light.constant 
			+ light.linear * distance
			+ light.quadratic * distance * distance);

	vec3	result = (ambient + diffuse + specular) * attenuation;
	color	= vec4(result , 1.0f);
}