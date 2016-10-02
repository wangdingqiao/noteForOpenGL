#version 330 core

layout(points) in ;
layout(line_strip, max_vertices = 2) out; // 注意输出类型

// 通过点产生直线输出
void main()
{
	gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
	gl_PointSize = gl_in[0].gl_PointSize;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
	EmitVertex();

	EndPrimitive();
}