#version 330 core

layout(triangles) in ;  // 输入三角形
layout(line_strip, max_vertices = 6) out;  // 输出3个代表法向量的直线

// 定义输入interface block
in VS_OUT
{
   vec3 normal;
}gs_in[];

float magnitude = 0.1f;

// 为指定索引的顶点产生代表法向量的直线
void generateNormalLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  EmitVertex();
  vec4 offset = vec4(gs_in[index].normal * magnitude, 0.0f);
  gl_Position = gl_in[index].gl_Position + offset;
  EmitVertex();
  EndPrimitive();
}

// 输出代表法向量的直线
void main()
{
	generateNormalLine(0);
	generateNormalLine(1);
	generateNormalLine(2);
}