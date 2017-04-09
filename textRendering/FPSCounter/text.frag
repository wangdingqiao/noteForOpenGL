#version 330

uniform vec3 textColor;
uniform sampler2D fontText;

in vec2 TextCoord;
out vec4 FragColor;

void main()
{   
   vec4 sample = vec4(1.0f,1.0f,1.0f,texture(fontText, TextCoord).r);
   FragColor = vec4(textColor, 1.0f) * sample;
   gl_FragDepth = 0.0f; // 设置为最小深度 总是保持文字在场景最前面
}