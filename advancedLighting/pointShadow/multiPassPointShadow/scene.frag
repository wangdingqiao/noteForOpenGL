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
uniform samplerCube depthCubeMap;  // 使用cubemap作为depth map
uniform sampler2D diffuseText;
uniform bool bUseShadow;
uniform bool bUsePCF;
uniform float far_plane;

out vec4 color;

// 通过depth-map和在世界坐标系位置 决定阴影
float calculateShadowSimple(vec3 fragPosInWordSpace)
{
	vec3 lightToFragDir = fragPosInWordSpace - light.position;
	float closestDepth = texture(depthCubeMap, lightToFragDir).r; // 使用一个向量索引cubemap深度纹理
	closestDepth *= far_plane;  // 从[0,1]恢复到原始水平
	float currentDepth = length(lightToFragDir);
	return currentDepth - 0.05 > closestDepth ? 1.0 : 0.0; // 使用偏置量修复shadow-acne
}
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// 使用了 percentage-closer filtering 制造软阴影
float calculateShadowPCF(vec3 fragPosInWordSpace)
{
    vec3 fragToLight = fragPosInWordSpace - light.position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPosInWordSpace);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthCubeMap,
		 fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    return shadow;
}

float calculateShadow(vec3 fragPosInWordSpace)
{
	if(bUsePCF)
	{
		return calculateShadowPCF(fragPosInWordSpace);
	}
	else
	{
	   return calculateShadowSimple(fragPosInWordSpace);
	}
}


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
	float shadow = 0.0f;
	shadow = bUseShadow ? calculateShadow(fs_in.FragPos) : 0.0;
	vec3	result = (ambient + (1.0 - shadow) * (diffuse + specular )) * objectColor;
	color	= vec4(result , 1.0f);
}