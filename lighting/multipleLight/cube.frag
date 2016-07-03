#version 330

in vec3 FragPos;
in vec2 TextCoord;
in vec3 FragNormal;

out vec4 color;

// 材质属性结构体
struct MaterialAttr
{
	sampler2D diffuseMap;	// 使用纹理对象根据片元位置取不同的材质属性
	sampler2D specularMap;
	float shininess; //镜面高光系数
};

// 方向光源属性结构体
struct DirLightAttr
{
	vec3 direction;	// 方向光源
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
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
// 聚光灯光源属性结构体
struct SpotLightAttr
{
	vec3 position;	// 聚光灯的位置
	vec3 direction; // 聚光灯的spot direction
	float cutoff;	// 聚光灯内部张角的余弦值
	float outerCutoff;	// 聚光灯外沿张角的余弦值
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;	// 衰减常数
	float linear;   // 衰减一次系数
	float quadratic; // 衰减二次系数
};

uniform MaterialAttr material;
uniform DirLightAttr dirLight;
#define POINT_LIGHT_NUM 4
uniform PointLightAttr pointLights[POINT_LIGHT_NUM]; // 定义点光源数组
uniform SpotLightAttr spotLight;
uniform vec3 viewPos;

// 计算光源效果的函数声明
vec3 calculateDirLight(DirLightAttr light, vec3 fragNormal, vec3 fragPos,vec3 viewPos);
vec3 calculatePointLight(PointLightAttr light, vec3 fragNormal, vec3 fragPos,vec3 viewPos);
vec3 calculateSpotLight(SpotLightAttr light,vec3 fragNormal, vec3 fragPos, vec3 viewPos);

void main()
{   
	vec3 result = calculateDirLight(dirLight, FragNormal,FragPos, viewPos);
	for(int i = 0; i < POINT_LIGHT_NUM; ++i)
	{
		result += calculatePointLight(pointLights[i],FragNormal, FragPos, viewPos);
	}
	result += calculateSpotLight(spotLight, FragNormal, FragPos, viewPos);
	color	= vec4(result , 1.0f);
}
// 方向光源计算方法
vec3 calculateDirLight(DirLightAttr light, vec3 fragNormal, vec3 fragPos,vec3 viewPos)
{
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	normal = normalize(fragNormal);
	vec3	lightDir = normalize(-light.direction);	// 翻转方向光源的方向
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(material.diffuseMap, TextCoord));

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - fragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3	specular = specFactor * light.specular * vec3(texture(material.specularMap, TextCoord));

	vec3	result = ambient + diffuse + specular;
	return result;
}
// 点光源计算方法

vec3 calculatePointLight(PointLightAttr light, vec3 fragNormal, vec3 fragPos,vec3 viewPos)
{
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - fragPos);
	vec3	normal = normalize(fragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(material.diffuseMap, TextCoord));

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - fragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3	specular = specFactor * light.specular * vec3(texture(material.specularMap, TextCoord));

	// 计算衰减因子
	float distance = length(light.position - FragPos); // 在世界坐标系中计算距离
	float attenuation = 1.0f / (light.constant 
			+ light.linear * distance
			+ light.quadratic * distance * distance);

	vec3	result = (ambient + diffuse + specular) * attenuation;
	return result;
}
// 聚光灯计算方法
vec3 calculateSpotLight(SpotLightAttr light,vec3 fragNormal, vec3 fragPos, vec3 viewPos)
{
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, TextCoord));

	vec3	lightDir = normalize(light.position - fragPos);
	vec3	normal = normalize(fragNormal);

	// 漫反射光成分 此时需要光线方向为指向光源
	
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(material.diffuseMap, TextCoord));

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - fragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3	specular = specFactor * light.specular * vec3(texture(material.specularMap, TextCoord));
	
	// 计算内外张角范围内的强度
	float theta = dot(lightDir, normalize(-light.direction));// 光线与聚光灯spotDir夹角余弦值
	float epsilon = light.cutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0); // 引入聚光灯内张角和外张角后的强度值
	diffuse *= intensity;
	specular *= intensity;

	// 计算衰减因子
	float distance = length(light.position - fragPos); // 在世界坐标系中计算距离
	float attenuation = 1.0f / (light.constant 
			+ light.linear * distance
			+ light.quadratic * distance * distance);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = ambient + diffuse + specular;
	return result;
}