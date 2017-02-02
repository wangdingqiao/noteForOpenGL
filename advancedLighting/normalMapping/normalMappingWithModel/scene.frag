#version 330

// 定义输入interface block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;// 保留这个用于对比方法的效果
	// 统一转换到tangent space计算
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
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
uniform bool normalMapping;
// 模型中的纹理
uniform sampler2D texture_diffuse0; 
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;

out vec4 color;

void main()
{   
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3    lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	vec3	normal = normalize(fs_in.FragNormal);
	if(normalMapping)  // 使用normal mapping时 从normal map取出normal vector
	{
		normal = texture(texture_normal0, fs_in.TextCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0); // 从颜色范围[0,1]还原到向量范围[-1,1]
	}
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	vec3    viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	float	specFactor = 0.0;
	vec3	halfDir = normalize(lightDir + viewDir);
	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0); // 64.0为镜面高光系数
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, fs_in.TextCoord));;

	vec3	result = (ambient + diffuse + specular );
	color	= vec4(result , 1.0f);
}