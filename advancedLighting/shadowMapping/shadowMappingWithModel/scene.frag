#version 330 core

// 定义输入interface block
in VS_OUT
{
	vec3 FragPos;
	vec2 TextCoord;
	vec3 FragNormal;
	vec4 FragPosInLightSpace;
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
uniform sampler2D depthMap;
uniform bool bUseShadow;
uniform bool bUsePCF;
// 修改这里以适应模型的绘制
uniform sampler2D texture_diffuse0; 
uniform sampler2D texture_specular0;

out vec4 color;

// 通过depth-map和在光源坐标系的位置 决定阴影
float calculateShadow1(vec4 fragPosInLightSpace)
{
	vec3 ndcPos = fragPosInLightSpace.xyz / fragPosInLightSpace.w;  // 从裁剪坐标系转换到NDC
	vec3 winPos = ndcPos * 0.5 + 0.5; // 从NDC变换到屏幕坐标系 实际上这里是纹理坐标的调整
	float closestDepth = texture(depthMap, winPos.xy).r; // 使用xy坐标索引深度纹理
	float currentDepth = winPos.z;
	return currentDepth > closestDepth ? 1.0 : 0.0;
}
// 使用偏置量修复shadow-acne
float calculateShadow2(vec4 fragPosInLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 ndcPos = fragPosInLightSpace.xyz / fragPosInLightSpace.w;  // 从裁剪坐标系转换到NDC
	vec3 winPos = ndcPos * 0.5 + 0.5; // 从NDC变换到屏幕坐标系 实际上这里是纹理坐标的调整
	float closestDepth = texture(depthMap, winPos.xy).r; // 使用xy坐标索引深度纹理
	float currentDepth = winPos.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005f); // 使用dot计算合理偏置量
	return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

// 使用偏置量修复shadow-acne 同时将超出light frustum的部分去掉阴影
float calculateShadow3(vec4 fragPosInLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 ndcPos = fragPosInLightSpace.xyz / fragPosInLightSpace.w;  // 从裁剪坐标系转换到NDC
	vec3 winPos = ndcPos * 0.5 + 0.5; // 从NDC变换到屏幕坐标系 实际上这里是纹理坐标的调整
	float closestDepth = texture(depthMap, winPos.xy).r; // 使用xy坐标索引深度纹理
	float currentDepth = winPos.z;
	if(currentDepth > 1.0)  // 这部分在far plane外面 不需要阴影
	{
	   return 0.0f;
	}

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005f); // 使用dot计算合理偏置量
	return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}
// 使用偏置量修复shadow-acne 同时将超出light frustum的部分去掉阴影
// 同时使用了 percentage-closer filtering 制造软阴影
float calculateShadow4(vec4 fragPosInLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 ndcPos = fragPosInLightSpace.xyz / fragPosInLightSpace.w;  // 从裁剪坐标系转换到NDC
	vec3 winPos = ndcPos * 0.5 + 0.5; // 从NDC变换到屏幕坐标系 实际上这里是纹理坐标的调整
	float currentDepth = winPos.z;
	if(currentDepth > 1.0)  // 这部分在far plane外面 不需要阴影
	{
	   return 0.0f;
	}

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005f); // 使用dot计算合理偏置量
	vec2 texelSize = 1.0 / textureSize(depthMap,0);
	float shadow = 0.0f;
	for(int x = -1; x <= 1;++x)
	{
		for(int y = -1; y <= 1;++y)
		{
		    float pcfDepth = texture(depthMap, winPos.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow = shadow / 9.0f;
	return shadow;
}

void main()
{   
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - fs_in.FragPos);
	vec3	normal = normalize(fs_in.FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	// 镜面光成分 使用Blinn-Phong模型
	vec3	viewDir = normalize(viewPos - fs_in.FragPos);
	vec3	halfDir = normalize(lightDir + viewDir);
	float	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); // 32.0为镜面高光系数
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, fs_in.TextCoord));
	float shadow = 0.0f;
	if(bUsePCF)
	{
		shadow = bUseShadow ? calculateShadow4(fs_in.FragPosInLightSpace, normal, lightDir) : 0.0;
	}
	else
	{
		shadow = bUseShadow ? calculateShadow3(fs_in.FragPosInLightSpace, normal, lightDir) : 0.0;
	}
	vec3	result = (ambient + (1.0 - shadow) * (diffuse + specular ));
	color	= vec4(result , 1.0f);
}