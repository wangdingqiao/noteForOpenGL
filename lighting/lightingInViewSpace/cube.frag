#version 330

in vec3 FragPos;
in vec2 TextCoord;
in vec3 FragNormal;
in vec3 LightPos;

out vec4 color;


uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{   
	// 环境光成分
	float	ambientStrength = 0.1f;
	vec3	ambient = ambientStrength * lightColor;

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(LightPos - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * lightColor;

	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(-FragPos); // 相机坐标系中viewPos位于原点 viewPos - FragPos = - FragPos 
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 32); // 32为镜面高光系数
	vec3	specular = specularStrength * specFactor * lightColor;

	vec3	result = (ambient + diffuse + specular ) * objectColor;
	color	= vec4(result , 1.0f);
}