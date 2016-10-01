// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
// 引入SOIL库
#include <SOIL/SOIL.h>
// 引入GLM库
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cstdlib>
// 包含着色器加载库
#include "shader.h"
// 包含相机控制辅助类
#include "camera.h"
// 包含纹理加载类
#include "texture.h"

// 键盘回调函数原型声明
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
// 鼠标移动回调函数原型声明
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
// 鼠标滚轮回调函数原型声明
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// 场景中移动
void do_movement();

// 定义程序常量
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
// 用于相机交互参数
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouseMove = true;
bool keyPressedStatus[1024]; // 按键情况记录
GLfloat deltaTime = 0.0f; // 当前帧和上一帧的时间差
GLfloat lastFrame = 0.0f; // 上一帧时间
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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
		"Demo of Using UBO in shader(layout=shared)", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	// 创建的窗口的context指定为当前context
	glfwMakeContextCurrent(window);

	// 注册窗口键盘事件回调函数
	glfwSetKeyCallback(window, key_callback);
	// 注册鼠标事件回调函数
	glfwSetCursorPosCallback(window, mouse_move_callback);
	// 注册鼠标滚轮事件回调函数
	glfwSetScrollCallback(window, mouse_scroll_callback);
	// 鼠标捕获 停留在程序内
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 初始化GLEW 获取OpenGL函数
	glewExperimental = GL_TRUE; // 让glew获取所有拓展函数
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION) 
			<< " error string:" << glewGetErrorString(status) << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	// 设置视口参数
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	//Section1 顶点属性数据
	// 指定立方体顶点属性数据 顶点位置 法向量
	GLfloat cubeVertices[] = {
		// 背面 ADC CBA
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

		// 正面EFG GHE
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

		// 左侧面 HDA AEH
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

		// 右侧面 GFB BCG
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		
		// 底面 ABF FEA
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

		// 顶面 DHG GCD
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
	};


	// Section2 准备缓存对象
	GLuint cubeVAOId, cubeVBOId;
	glGenVertexArrays(1, &cubeVAOId);
	glGenBuffers(1, &cubeVBOId);
	glBindVertexArray(cubeVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	// 顶点位置数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		6 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点法向量数据
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
		6 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Section3 准备着色器程序
	Shader redShader("ubo_vs.vertex", "ubo_red.frag");
	Shader greenShader("ubo_vs.vertex", "ubo_green.frag");
	Shader blueShader("ubo_vs.vertex", "ubo_blue.frag");
	Shader yellowShader("ubo_vs.vertex", "ubo_yellow.frag");

	// Section4 创建std140 layout UBO 并将shader中 uniform buffer和程序中UBO对应起来
	// step1 获取shader中 uniform buffer 的索引
	GLuint redShaderIndex = glGetUniformBlockIndex(redShader.programId, "Matrices");
	GLuint greeShaderIndex = glGetUniformBlockIndex(greenShader.programId, "Matrices");
	GLuint blueShaderIndex = glGetUniformBlockIndex(blueShader.programId, "Matrices");
	GLuint yellowShaderIndex = glGetUniformBlockIndex(yellowShader.programId, "Matrices");
	// step2 设置shader中 uniform buffer 的索引到指定绑定点
	glUniformBlockBinding(redShader.programId, redShaderIndex, 0); // 绑定点为0
	glUniformBlockBinding(greenShader.programId, greeShaderIndex, 0);
	glUniformBlockBinding(blueShader.programId, blueShaderIndex, 0);
	glUniformBlockBinding(yellowShader.programId, yellowShaderIndex, 0);
	// step3 创建UBO 并链接到指定绑定点
	GLuint UBOId;
	glGenBuffers(1, &UBOId);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOId);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW); // 预分配空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOId, 0, 2 * sizeof(glm::mat4)); // 绑定点为0
	// step4 只更新一部分值 投影矩阵 假设投影矩阵不变
	glBindBuffer(GL_UNIFORM_BUFFER, UBOId);
	glm::mat4 projection = glm::perspective(45.0f,
		(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f); // 投影矩阵
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Section5 再添加一个 share layout 的UBO
	redShaderIndex = glGetUniformBlockIndex(redShader.programId, "mixColorSettings");
	greeShaderIndex = glGetUniformBlockIndex(greenShader.programId, "mixColorSettings");
	blueShaderIndex = glGetUniformBlockIndex(blueShader.programId, "mixColorSettings");
	yellowShaderIndex = glGetUniformBlockIndex(yellowShader.programId, "mixColorSettings");
	// step1 设置shader中 uniform buffer 的索引到指定绑定点
	glUniformBlockBinding(redShader.programId, redShaderIndex, 1); // 绑定点为1
	glUniformBlockBinding(greenShader.programId, greeShaderIndex, 1);
	glUniformBlockBinding(blueShader.programId, blueShaderIndex, 1);
	glUniformBlockBinding(yellowShader.programId, yellowShaderIndex, 1);
	// step2 创建UBO 并链接到指定绑定点
	GLuint colorUBOId;
	glGenBuffers(1, &colorUBOId);
	glBindBuffer(GL_UNIFORM_BUFFER, colorUBOId);
	// 获取UBO大小 因为定义相同 只需要在一个shader中获取大小即可
	GLint blockSize;
	glGetActiveUniformBlockiv(redShader.programId, redShaderIndex,
		GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
	if (blockSize <= 0)
	{
		std::cerr << "Error::glGetActiveUniformBlockiv, could not get ubo block size." << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	glBufferData(GL_UNIFORM_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW); // 预分配空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, colorUBOId); // 绑定点为1
	// step3 更新数据
	// 通过查询获取uniform buffer中各个变量的索引和位移偏量
	const GLchar* names[] = {
		"anotherColor", "mixValue"
	};
	GLuint indices[2];
	glGetUniformIndices(redShader.programId, 2, names, indices);
	GLint offset[2];
	glGetActiveUniformsiv(redShader.programId, 2, indices, GL_UNIFORM_OFFSET, offset);
	for (int i = 0; i < 2; ++i)
	{
		std::cout << "Get uniform buffer variable names: " << names[i] 
			<< " offset: " << offset[i] << std::endl;
	}
	// 使用获取的位移偏量更新数据
	glm::vec4 anotherColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	GLfloat mixValue = 0.5f;
	glBindBuffer(GL_UNIFORM_BUFFER, colorUBOId);
	glBufferSubData(GL_UNIFORM_BUFFER, offset[0], sizeof(glm::vec4), glm::value_ptr(anotherColor));
	glBufferSubData(GL_UNIFORM_BUFFER, offset[1], sizeof(glm::vec4), &mixValue);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性

		// 设置colorBuffer颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		// 清除colorBuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 更新部分UBO数据
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, UBOId);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), 
			sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// 利用4个shader绘制4个立方体
		glBindVertexArray(cubeVAOId);
		// 绘制红色
		redShader.use();
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f)); // 左上角
		glUniformMatrix4fv(glGetUniformLocation(redShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 绘制绿色
		greenShader.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f)); // 右上角
		glUniformMatrix4fv(glGetUniformLocation(greenShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 绘制黄色
		yellowShader.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f)); // 左下角
		
		glUniformMatrix4fv(glGetUniformLocation(yellowShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 绘制蓝色
		blueShader.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f)); // 右下角
		glUniformMatrix4fv(glGetUniformLocation(blueShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		
		glBindVertexArray(0);
		glUseProgram(0);
		glDepthFunc(GL_LESS);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &cubeVAOId);
	glDeleteBuffers(1, &cubeVBOId);
	glDeleteBuffers(1, &UBOId);
	glfwTerminate();
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keyPressedStatus[key] = true;
		else if (action == GLFW_RELEASE)
			keyPressedStatus[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
}
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMove) // 首次鼠标移动
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false; 
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.handleMouseMove(xoffset, yoffset);
}
// 由相机辅助类处理鼠标滚轮控制
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.handleMouseScroll(yoffset);
}
// 由相机辅助类处理键盘控制
void do_movement()
{
	
	if (keyPressedStatus[GLFW_KEY_W])
		camera.handleKeyPress(FORWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_S])
		camera.handleKeyPress(BACKWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_A])
		camera.handleKeyPress(LEFT, deltaTime);
	if (keyPressedStatus[GLFW_KEY_D])
		camera.handleKeyPress(RIGHT, deltaTime);
}