#version 330 core

layout (triangles) in;  // 输入三角形
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];
out vec4 FragPos; // 输出世界坐标系位置

void main()
{
    // 一共6个cubemap面
	for(int face = 0; face < 6;++face)
	{
		gl_Layer = face;  // 设置当前计算的面

		// 为每个输入的三角形顶点 计算点光源对应的6个面的坐标系中位置
		for(int i=0; i < 3;++i)
		{
			FragPos = gl_in[i].gl_Position;
			gl_Position = shadowMatrices[face] * FragPos; // 计算每个顶点在光源为视点的裁剪坐标系中位置
			EmitVertex();
		}
		EndPrimitive();
	}
}