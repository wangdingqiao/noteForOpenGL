#version 330 core

in vec3 FragPos;
in vec2 TextCoord;
in vec3 FragNormal;

// 点光源属性结构体
struct PointLightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;	// 衰减常数
	float linear;   // 衰减一次系数
	float quadratic; // 衰减二次系数
};

#define LIGHT_NUM 2
uniform PointLightAttr lights[LIGHT_NUM];
uniform vec3 viewPos;
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular0;
uniform sampler2D texture_specular1;
uniform bool bGamma;

out vec4 color;

vec3 calcPointLight(PointLightAttr light)
{
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse0, TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, TextCoord));

	// 镜面反射成分 采用Blinn-Phong模型
	float	specularStrength = 0.5f;
	vec3	viewDir = normalize(viewPos - FragPos);
	vec3    halfDir = normalize(lightDir + viewDir);
	float	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0f);
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, TextCoord));

	// 计算衰减因子
	float distance = length(light.position - FragPos); // 在世界坐标系中计算距离
	float attenuation = 1.0f / (light.constant 
			+ light.linear * distance
			+ light.quadratic * distance * distance);

	vec3	result = (ambient + diffuse + specular) * attenuation;
	return result;
}

void main()
{
	vec3 result = vec3(0.0);
	for(int i = 0; i < LIGHT_NUM;++i)
	   result += calcPointLight(lights[i]);
	if(bGamma)
	{
		result = pow(result, vec3(1.0 / 2.2));	// 启用Gamma校正
	}
	color	= vec4(result , 1.0f);
}