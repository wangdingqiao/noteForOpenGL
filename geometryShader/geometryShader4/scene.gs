#version 330 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // 注意输出类型

// 定义输入interface block
in VS_OUT
{
   vec3 vertColor;
}gs_in[];

out vec3 fcolor;

void makeHouse(vec4 position)
{
    fcolor = gs_in[0].vertColor;
	gl_PointSize = gl_in[0].gl_PointSize;
    gl_Position = position + vec4(-0.2f, -0.2f, 0.0f, 0.0f);  // 左下角
	EmitVertex();
	gl_Position = position + vec4(0.2f, -0.2f, 0.0f, 0.0f);  // 右下角
	EmitVertex();
	gl_Position = position + vec4(-0.2f, 0.2f, 0.0f, 0.0f);  // 左上角
	EmitVertex();
	gl_Position = position + vec4(0.2f, 0.2f, 0.0f, 0.0f);  // 右上角
	EmitVertex();
	gl_Position = position + vec4(0.0f, 0.4f, 0.0f, 0.0f);  // 顶部
	fcolor = vec3(1.0f, 1.0f, 1.0f); // 这里改变顶部颜色
	EmitVertex();
	EndPrimitive();
}
// 输出房子样式三角形带
void main()
{
	makeHouse(gl_in[0].gl_Position);
}