#version 330

in vec2 TextCoord;

out vec4 color;

uniform sampler2D sceneText;
uniform sampler2D blurText;
uniform bool  bUseBloom;
uniform float explosureFactor;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor =	  texture(sceneText, TextCoord).rgb;
	vec3 bloomColor = texture(blurText, TextCoord).rgb;
	if(bUseBloom)
		hdrColor += bloomColor;
    // 从HDR到LDR的映射 tone mapping
	vec3 result = vec3(1.0) - exp(-hdrColor * explosureFactor);
	// gamma校正
	result = pow(result, vec3(1.0 / gamma));
	color = vec4(result, 1.0f);
}