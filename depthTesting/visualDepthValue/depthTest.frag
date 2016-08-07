#version 330 core

in vec2 TextCoord;
uniform sampler2D text;


out vec4 color;

// 原样输出
float asDepth()
{
  return gl_FragCoord.z;
}
// 线性输出结果
float near = 1.0f; 
float far  = 100.0f; 
float LinearizeDepth() 
{
    float Zndc = gl_FragCoord.z * 2.0 - 1.0; // 计算ndc坐标 这里默认glDepthRange(0,1)
    float Zeye = (2.0 * near * far) / (far + near - Zndc * (far - near)); // 这里分母进行了反转
	return (Zeye - near)/ ( far - near);
}
// 非线性输出
float nonLinearDepth()
{
	float Zndc = gl_FragCoord.z * 2.0 - 1.0; // 计算ndc坐标 这里默认glDepthRange(0,1)
    float Zeye = (2.0 * near * far) / (far + near - Zndc * (far - near)); // 这里分母进行了反转
	return (1.0 / near - 1.0 / Zeye) / (1.0 / near - 1.0 / far);
}
void main()
{
	float depth = LinearizeDepth();
	color = vec4(vec3(depth), 1.0f);
}