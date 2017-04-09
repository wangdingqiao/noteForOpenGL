#version 330 core

// 定义输入interface block
in VS_OUT
{
	vec3 FragPos;
	vec2 TextCoord;
	vec3 FragNormal;
}fs_in;

// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform LightAttr light;
uniform vec3 viewPos;
uniform sampler2D diffuseText;

out vec4 color;

void main()
{   
    vec3	objectColor = texture(diffuseText,fs_in.TextCoord).rgb;
	// 环境光成分
	vec3	ambient = light.ambient;

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - fs_in.FragPos);
	vec3	normal = normalize(fs_in.FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse;

	// 镜面光成分 使用Blinn-Phong模型
	vec3	viewDir = normalize(viewPos - fs_in.FragPos);
	vec3	halfDir = normalize(lightDir + viewDir);
	float	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
	vec3	specular = specFactor * light.specular;
	vec3	result = (ambient + diffuse + specular ) * objectColor;
	color	= vec4(result , 1.0f);
}