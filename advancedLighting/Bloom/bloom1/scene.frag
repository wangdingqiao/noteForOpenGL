#version 330
layout (location = 0) out vec4 FragColor;  // 指定输出到第一个color buffer
layout (location = 1) out vec4 BrightColor;// 指定输出到第二个color buffer

// 定义输入interface block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;
}fs_in;

// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform LightAttr lights[4];
uniform vec3 viewPos;
uniform sampler2D text;

//out vec4 FragColor;

void main()
{   
    vec3 objectColor = texture(text,fs_in.TextCoord).rgb;
	// 环境光成分
	vec3	ambitentStrength = vec3(0.1,0.1,0.1);
	float	ambientStrength = 0.1f;
	vec3	ambient = ambientStrength * ambitentStrength;
	vec3	lightSum = vec3(0.0f);
	for(int i = 0 ; i < 4; ++i)
	{
		
		// 漫反射光成分 此时需要光线方向为指向光源
		vec3	lightDir = normalize(lights[i].position - fs_in.FragPos);
		vec3	normal = normalize(fs_in.FragNormal);
		float	diffFactor = max(dot(lightDir, normal), 0.0);
		vec3	diffuse = diffFactor * lights[i].diffuse;

		// 镜面光成分 Bling-Phong模型
		vec3	viewDir = normalize(viewPos - fs_in.FragPos);
		vec3	halfDir = normalize(lightDir + viewDir);
		float	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
		vec3	specular = specFactor * lights[i].specular;
		
		// 衰减计算
		float distance = length(fs_in.FragPos - lights[i].position);
		float attenuation = 1.0 / (distance * distance);
		lightSum += (diffuse + specular) * attenuation;
	}
	vec3	result = (ambient + lightSum) * objectColor;
	FragColor = vec4(objectColor , 1.0f);
	// 根据高亮部分阈值 决定输出到高亮对应的color buffer
	float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.0f)
		BrightColor = vec4(result, 1.0f);
}