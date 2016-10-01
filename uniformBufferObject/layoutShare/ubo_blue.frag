#version 330 core

// 使用默认shared​方式的UBO
uniform mixColorSettings {
	vec4  anotherColor;
	float mixValue;
};

out vec4 color;


void main()
{
	color = mix(vec4(0.0, 0.0, 1.0, 1.0), anotherColor, mixValue);
}