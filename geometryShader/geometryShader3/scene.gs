#version 330 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // 注意输出类型

void makeHouse(vec4 position)
{
    gl_Position = position + vec4(-0.2f, -0.2f, 0.0f, 0.0f);  // 左下角
	EmitVertex();
	gl_Position = position + vec4(0.2f, -0.2f, 0.0f, 0.0f);  // 右下角
	EmitVertex();
	gl_Position = position + vec4(-0.2f, 0.2f, 0.0f, 0.0f);  // 左上角
	EmitVertex();
	gl_Position = position + vec4(0.2f, 0.2f, 0.0f, 0.0f);  // 右上角
	EmitVertex();
	gl_Position = position + vec4(0.0f, 0.4f, 0.0f, 0.0f);  // 顶部
	EmitVertex();
	EndPrimitive();
}
// 输出房子样式三角形带
void main()
{
    gl_PointSize = gl_in[0].gl_PointSize;
	makeHouse(gl_in[0].gl_Position);
}