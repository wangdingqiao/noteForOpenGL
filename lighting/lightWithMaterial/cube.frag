#version 330

in vec3 FragPos;
in vec2 TextCoord;
in vec3 FragNormal;

out vec4 color;

// 材质属性结构体
struct MaterialAttr
{
	vec3 ambient;	// 环境光
	vec3 diffuse;	 // 漫反射光
	vec3 specular;   // 镜面光
	float shininess; //镜面高光系数
};
// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform MaterialAttr material;
uniform LightAttr light;
uniform vec3 viewPos;

void main()
{   
	// 环境光成分
	vec3	ambient = light.ambient * material.ambient;

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * material.diffuse;

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - FragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3	specular = specFactor * light.specular * material.specular;

	vec3	result = ambient + diffuse + specular;
	color	= vec4(result , 1.0f);
}