#version 330

in vec2 TextCoord;

out vec4 FragColor;

// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float linear;   // 一次衰减系数
	float quadratic; // 二次衰减系数
};
#define NR_LIGHTS 32
uniform LightAttr lights[NR_LIGHTS];
uniform vec3 viewPos;

uniform sampler2D gPositonText;
uniform sampler2D gNormalText;
uniform sampler2D gAlbedoSpecText;

uniform int drawMode;


void main()
{
	// 从GBuffer中各个纹理取出片元的信息
	vec3 FragPos = texture(gPositonText, TextCoord).rgb;
	vec3 FragNormal = texture(gNormalText, TextCoord).rgb;
	vec3 DiffuseColor = texture(gAlbedoSpecText, TextCoord).rgb;
	float SpecStrength = texture(gAlbedoSpecText,TextCoord).a;
	
	// 环境光成分
	vec3	ambientColor = DiffuseColor;
	float	ambientStrength = 0.1f;
	vec3	ambient = ambientStrength * ambientColor;
	vec3	lightSum = ambient;
	vec3	viewDir = normalize(viewPos - FragPos);// 提到循环外面 避免重复计算
	for(int i = 0 ; i < NR_LIGHTS; ++i)
	{
		// 漫反射光成分 此时需要光线方向为指向光源
		vec3	lightDir = normalize(lights[i].position - FragPos);
		vec3	normal = normalize(FragNormal);
		float	diffFactor = max(dot(lightDir, normal), 0.0);
		vec3	diffuse = diffFactor * lights[i].diffuse * DiffuseColor;

		// 镜面光成分 Bling-Phong模型
		vec3	halfDir = normalize(lightDir + viewDir);
		float	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
		vec3	specular = SpecStrength * specFactor * lights[i].specular;
		
		// 衰减计算
		float distance = length(FragPos - lights[i].position);
		float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
		lightSum += (diffuse + specular) * attenuation;
	}
	if (drawMode == 1) 
		FragColor = vec4(lightSum, 1.0);
	else if(drawMode == 2)
	    FragColor = vec4(FragPos, 1.0);
	else if(drawMode == 3)
		FragColor = vec4(FragNormal, 1.0);
	else if(drawMode == 4)
		FragColor = vec4(DiffuseColor,1.0);
	else
	   FragColor = vec4(vec3(SpecStrength), 1.0);
}