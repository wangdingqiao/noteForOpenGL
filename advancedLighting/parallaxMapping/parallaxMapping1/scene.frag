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

uniform bool bParallaxMapping;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform float heightScale;
out vec4 color;

vec2 parallaxMapping(vec2 textCoord,vec3 viewDir)
{
	float height = texture(heightMap, textCoord).r;
	vec2  offset = viewDir.xy / viewDir.z * (height * heightScale);
	return textCoord - offset;
}

void main()
{   
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 textCoord = fs_in.TextCoord;
	if(bParallaxMapping)
	{
		textCoord = parallaxMapping(fs_in.TextCoord, viewDir);
		if(textCoord.x < 0.0 
		|| textCoord.y < 0.0 
		|| textCoord.x > 1.0 
		|| textCoord.y > 1.0)
			discard;
	}
    vec3 objectColor = texture(diffuseMap,textCoord).rgb;
	// 环境光成分
	float	ambientStrength = 0.1f;
	vec3	ambient = ambientStrength * light.ambient;

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3    lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	vec3	normal = texture(normalMap, textCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0); // 从颜色范围[0,1]还原到向量范围[-1,1]
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse;

	float	specFactor = 0.0;
	vec3 halfDir = normalize(lightDir + viewDir);
	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
	vec3	specular = specFactor * light.specular;

	vec3	result = (ambient + diffuse + specular ) * objectColor;
	color	= vec4(result , 1.0f);
}