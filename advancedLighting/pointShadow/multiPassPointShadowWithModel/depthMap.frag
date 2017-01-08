#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	// 不使用OpenGL自动输出的深度 在世界坐标系中计算深度
	// gl_FragDepth = gl_FragCoord.z; // 不使用默认方式

	 // 计算片元和光源位置
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // 调整到[0,1]范围
    lightDistance = lightDistance / far_plane;
    
    // 作为深度输出
    gl_FragDepth = lightDistance;
}