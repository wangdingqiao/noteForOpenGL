#version 330 core

layout(triangles) in ;  // 输入三角形
layout(triangle_strip, max_vertices = 3) out;

// 定义输入interface block
in VS_OUT
{
   vec2 TextCoord;
}gs_in[];

out vec2 TextCoord;
uniform float time; // 随时间进行explode效果

// 从输入的3个顶点 计算法向量
vec3 getNormal(vec4 pos0, vec4 pos1, vec4 pos2)
{
  vec3 a = vec3(pos0) - vec3(pos1);
  vec3 b = vec3(pos2) - vec3(pos1);
  return normalize(cross(a, b));
}

// 计算偏移后的三角形顶点
void explode()
{
  vec3 normal = getNormal(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
  float magnitude = ((sin(time) + 1) / 2.0f) * 2.0f; // 使位移偏量保持在[0, 2.0f]范围内
  vec4 offset = vec4(normal * magnitude, 0.0f);
  gl_Position = gl_in[0].gl_Position + offset;
  TextCoord = gs_in[0].TextCoord; // 顶点和纹理坐标每个顶点都不相同
  EmitVertex();
  gl_Position = gl_in[1].gl_Position + offset;
  TextCoord = gs_in[1].TextCoord;
  EmitVertex();
  gl_Position = gl_in[2].gl_Position + offset;
  TextCoord = gs_in[2].TextCoord;
  EmitVertex();
  EndPrimitive();
}

// 输出在法向量方向发生偏移的顶点位置 模拟爆炸效果
void main()
{
	explode();
}