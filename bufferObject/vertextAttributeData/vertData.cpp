// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
// 引入SOIL库
#include <SOIL/SOIL.h>

#include <iostream>
#include <vector>

// 包含着色器加载库
#include "shader.h"
// 包含纹理加载辅助类
#include "texture.h"

// 键盘回调函数原型声明
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// 定义程序常量
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
GLfloat colorMixValue = 0.5;

// 不同的顶点属性数据传送方式
// 这些例子主要用于展示函数用法 实际使用视具体情况而定
void prepareVertData_moreVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);
void prepareVertData_seqBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);
void prepareVertData_interleavedBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);
void prepareVertData_glBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);
void prepareVertData_glMapBuffer(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);
void prepareVertData_glCopyBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec);

int main(int argc, char** argv)
{
	
	if (!glfwInit())	// 初始化glfw库
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}

	// 开启OpenGL 3.3 core profile
	std::cout << "Start OpenGL core profile version 3.3" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// 创建窗口
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
		"Demo of vertex attribute data(press A S to mix color)", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		return -1;
	}
	// 创建的窗口的context指定为当前context
	glfwMakeContextCurrent(window);

	// 注册窗口键盘事件回调函数
	glfwSetKeyCallback(window, key_callback);

	// 初始化GLEW 获取OpenGL函数
	glewExperimental = GL_TRUE; // 让glew获取所有拓展函数
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION) 
			<< " error string:" << glewGetErrorString(status) << std::endl;
		glfwTerminate();
		return -1;
	}

	// 设置视口参数
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	// section1 准备顶点属性数据
	GLuint VAOId;
	std::vector<GLuint> VBOIdVec;
	prepareVertData_glCopyBufferSubData(VAOId, VBOIdVec); // 替换这个方法以观察其他方式
	// Section2 准备着色器程序
	Shader shader("triangle.vertex", "triangle.frag");

	// Section3 准备纹理对象
	GLint textureId = TextureHelper::load2DTexture("../../resources/textures/window.png");

	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // 处理例如鼠标 键盘等事件

		// 清除颜色缓冲区 重置为指定颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 这里填写场景绘制代码
		glBindVertexArray(VAOId);
		shader.use();
		// 启用纹理单元 绑定纹理对象
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(glGetUniformLocation(shader.programId, "tex"), 0); // 设置纹理单元为0号
		glUniform1f(glGetUniformLocation(shader.programId, "colorMixValue"), colorMixValue);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &VAOId);
	for (std::vector<GLuint>::const_iterator it = VBOIdVec.begin();
		VBOIdVec.end() != it; ++ it)
	{
		glDeleteBuffers(1, &(*it));
	}
	glfwTerminate();
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		colorMixValue += 0.05f;
		if (colorMixValue > 1.0f)
			colorMixValue = 1.0f;
	}
	else if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		colorMixValue -= 0.05f;
		if (colorMixValue < 0.0f)
			colorMixValue = 0.0f;
	}
}
// 顶点属性每个独立的VBO
void prepareVertData_moreVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性独立
	GLfloat vertPos[] = {
		-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
	};
	GLfloat vertColor[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
	};
	GLfloat vertTextCoord[] = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
	};
	// 创建缓存对象
	GLuint VBOId[3];
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	glGenBuffers(3, VBOId);
	// 顶点位置属性
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos), vertPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertColor), vertColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertTextCoord), vertTextCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId[0]);
	VBOIdVec.push_back(VBOId[1]);
	VBOIdVec.push_back(VBOId[2]);
}
// 顶点属性单个连续的整体VBO
void prepareVertData_seqBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性连续作为整体
	GLfloat vertices[] = {
		-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 传送数据
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Step3: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 紧密排列 stride也可以设置为 3 * sizeof(GL_FLOAT)
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(18 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}

// 顶点属性之间交错的整体VBO
void prepareVertData_interleavedBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
		// 顶点位置 颜色 纹理纹理
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 传送数据
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Step3: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT),(GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}
// 使用glBufferSubData完成VBO
void prepareVertData_glBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性独立
	GLfloat vertPos[] = {
		-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
	};
	GLfloat vertColor[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
	};
	GLfloat vertTextCoord[] = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 预分配空间
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos)+sizeof(vertColor) + sizeof(vertTextCoord), 
		NULL, GL_STATIC_DRAW);  // 预分配空间
	// Step3: 填充具体的数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertPos), vertPos);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos), sizeof(vertColor), vertColor);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos)+ sizeof(vertColor), sizeof(vertTextCoord), vertTextCoord);
	// Step4: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 紧密排列 stride也可以设置为 3 * sizeof(GL_FLOAT)
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(18 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}

// 使用glMapBuffer
void prepareVertData_glMapBuffer(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
		// 顶点位置 颜色 纹理纹理
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	// Step3: 映射数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 预分配空间
	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!ptr)
	{
		std::cerr << " map buffer failed!" << std::endl;
		return;
	}
	memcpy(ptr, vertices, sizeof(vertices));  // 可以操作这个指针指向的内存空间
	glUnmapBuffer(GL_ARRAY_BUFFER);
	// Step4: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}

// 使用glCopyBufferSubData 
void prepareVertData_glCopyBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
		// 顶点位置 颜色 纹理纹理
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	GLuint TempBufferId;
	glGenBuffers(1, &TempBufferId);
	glBindBuffer(GL_COPY_WRITE_BUFFER, TempBufferId);
	// Step3: 映射数据到GL_COPY_WRITE_BUFFER
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 预分配空间
	void* ptr = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	if (!ptr)
	{
		std::cerr << " map buffer failed!" << std::endl;
		return;
	}
	memcpy(ptr, vertices, sizeof(vertices));  // 可以操作这个指针指向的内存空间
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	// Step4: 将数据拷贝到VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 注意拷贝前预分配空间
	glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_ARRAY_BUFFER, 0, 0, sizeof(vertices));
	glDeleteBuffers(1, &TempBufferId);
	// Step5: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}