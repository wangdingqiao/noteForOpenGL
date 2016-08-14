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
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

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
		"Demo of Planar Reflection(using stencil buffer)", NULL, NULL);
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
	// 指定立方体顶点属性数据 顶点位置 颜色 纹理
	GLfloat cubeVertices[] = {
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,	// A
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,	// B
		0.5f, 0.5f, 0.5f,1.0f, 1.0f,	// C
		0.5f, 0.5f, 0.5f,1.0f, 1.0f,	// C
		-0.5f, 0.5f, 0.5f,0.0f, 1.0f,	// D
		-0.5f, -0.5f, 0.5f,0.0f, 0.0f,	// A


		-0.5f, -0.5f, -0.5f,0.0f, 0.0f,	// E
		-0.5f, 0.5f, -0.5f,0.0, 1.0f,   // H
		0.5f, 0.5f, -0.5f,1.0f, 1.0f,	// G
		0.5f, 0.5f, -0.5f,1.0f, 1.0f,	// G
		0.5f, -0.5f, -0.5f,1.0f, 0.0f,	// F
		-0.5f, -0.5f, -0.5f,0.0f, 0.0f,	// E

		-0.5f, 0.5f, 0.5f,0.0f, 1.0f,	// D
		-0.5f, 0.5f, -0.5f,1.0, 1.0f,   // H
		-0.5f, -0.5f, -0.5f,1.0f, 0.0f,	// E
		-0.5f, -0.5f, -0.5f,1.0f, 0.0f,	// E
		-0.5f, -0.5f, 0.5f,0.0f, 0.0f,	// A
		-0.5f, 0.5f, 0.5f,0.0f, 1.0f,	// D

		0.5f, -0.5f, -0.5f,1.0f, 0.0f,	// F
		0.5f, 0.5f, -0.5f,1.0f, 1.0f,	// G
		0.5f, 0.5f, 0.5f,0.0f, 1.0f,	// C
		0.5f, 0.5f, 0.5f,0.0f, 1.0f,	// C
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f,	// B
		0.5f, -0.5f, -0.5f,1.0f, 0.0f,	// F

		0.5f, 0.5f, -0.5f,1.0f, 1.0f,	// G
		-0.5f, 0.5f, -0.5f,0.0, 1.0f,   // H
		-0.5f, 0.5f, 0.5f,0.0f, 0.0f,	// D
		-0.5f, 0.5f, 0.5f,0.0f, 0.0f,	// D
		0.5f, 0.5f, 0.5f,1.0f, 0.0f,	// C
		0.5f, 0.5f, -0.5f,1.0f, 1.0f,	// G

		-0.5f, -0.5f, 0.5f,0.0f, 0.0f,	// A
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,// E
		0.5f, -0.5f, -0.5f,1.0f, 1.0f,	// F
		0.5f, -0.5f, -0.5f,1.0f, 1.0f,	// F
		0.5f, -0.5f, 0.5f,1.0f, 0.0f,	// B
		-0.5f, -0.5f, 0.5f,0.0f, 0.0f,	// A
	};
	// 模拟镜面的顶点属性数据 顶点位置 纹理坐标(设置的值大于1.0用于重复)
	GLfloat planeVertices[] = {
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,   // A
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f,  // D
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f, // C

		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f, // C
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,  // B
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,   // A
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
		5 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点纹理数据
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
		5 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	GLuint planeVAOId, planeVBOId;
	glGenVertexArrays(1, &planeVAOId);
	glGenBuffers(1, &planeVBOId);
	glBindVertexArray(planeVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	// 顶点位置数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		5 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点纹理数据
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		5 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Section3 加载纹理
	GLuint cubeTextId = TextureHelper::load2DTexture("../../resources/textures/marble.jpg");
	// Section4 准备着色器程序
	Shader shader("stencilTest.vertex", "stencilTest.frag");
	Shader planeShader("singleColor.vertex", "singleColor.frag");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性

		// 清除颜色缓冲区 深度缓冲区 模板缓冲区
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 100.0f); // 投影矩阵
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵

		// section1 利用反射平面绘制模板
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 在模板测试和深度测试都通过时更新模板缓冲区

		planeShader.use();
		glUniformMatrix4fv(glGetUniformLocation(planeShader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(planeShader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model;
		glUniformMatrix4fv(glGetUniformLocation(planeShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAOId);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// section2 绘制反射部分
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glStencilMask(0x00); // 禁止写入stencil
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(cubeVAOId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTextId);
		// 绘制第一个立方体反射
		glm::mat4 firstReflectModel;
		firstReflectModel = glm::translate(firstReflectModel, glm::vec3(-1.0f, 0.0f, -1.0f));
		firstReflectModel = glm::translate(firstReflectModel, glm::vec3(0.0f, -0.5f, 0.0f));
		firstReflectModel = glm::scale(firstReflectModel, glm::vec3(1.0f, -1.0f, 1.0f));
		firstReflectModel = glm::translate(firstReflectModel, glm::vec3(0.0f, 0.5f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(firstReflectModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 绘制第二个立方体反射
		glm::mat4 secondReflectModel;
		secondReflectModel = glm::translate(secondReflectModel, glm::vec3(2.0f, 0.0f, 0.0f));
		secondReflectModel = glm::translate(secondReflectModel, glm::vec3(0.0f, -0.5f, 0.0f));
		secondReflectModel = glm::scale(secondReflectModel, glm::vec3(1.0f, -1.0f, 1.0f));
		secondReflectModel = glm::translate(secondReflectModel, glm::vec3(0.0f, 0.5f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(secondReflectModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glDisable(GL_STENCIL_TEST); // 绘制平面和原始立方体时关闭模板测试
		// section3 绘制反射平面
		planeShader.use();
		glBindVertexArray(planeVAOId);
		glEnable(GL_BLEND); // 为反射平面启用透明特性
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisable(GL_BLEND);

		// section4 绘制原始立方体
		shader.use();
		glBindVertexArray(cubeVAOId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTextId);
		// 绘制第一个原始立方体
		glm::mat4 firstCubeModel;
		firstCubeModel = glm::translate(firstCubeModel, glm::vec3(-1.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(firstCubeModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 绘制第二个原始立方体
		glm::mat4 secondCubeModel;
		secondCubeModel = glm::translate(secondCubeModel, glm::vec3(2.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(secondCubeModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);
		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &cubeVAOId);
	glDeleteVertexArrays(1, &planeVAOId);
	glDeleteBuffers(1, &cubeVBOId);
	glDeleteBuffers(1, &planeVBOId);
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