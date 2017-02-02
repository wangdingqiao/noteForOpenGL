#version 330

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
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform LightAttr light;
uniform vec3 viewPos;
uniform bool normalMapping;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

out vec4 color;

void main()
{   
    vec3 objectColor = texture(diffuseMap,fs_in.TextCoord).rgb;
	// 环境光成分
	float	ambientStrength = 0.1f;
	vec3	ambient = ambientStrength * light.ambient;

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - fs_in.FragPos);
	vec3	normal = normalize(fs_in.FragNormal);
	if(normalMapping)  // 使用normal mapping时 从normal map取出normal vector
	{
		normal = texture(normalMap, fs_in.TextCoord).rgb;
        normal = normalize(normal * 2.0 - 1.0); // 从颜色范围[0,1]还原到向量范围[-1,1]
	}
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse;

	vec3	viewDir = normalize(viewPos - fs_in.FragPos);
	float	specFactor = 0.0;
	vec3 halfDir = normalize(lightDir + viewDir);
	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
	vec3	specular = specFactor * light.specular;

	vec3	result = (ambient + diffuse + specular ) * objectColor;
	color	= vec4(result , 1.0f);
}