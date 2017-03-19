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
#include <sstream>

// 包含着色器加载库
#include "shader.h"
// 包含相机控制辅助类
#include "camera.h"
// 包含纹理加载辅助类
#include "texture.h"
// 包含字体管理类
#include "font.h"

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
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

GLuint quadVAOId, quadVBOId;
void prepareVBO();
void renderText(Shader &shader, std::wstring text, 
	GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
std::map<wchar_t, FontCharacter> unicodeCharacters;

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
		"Demo of Text Rendering", NULL, NULL);
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
		return -1;
	}

	// 设置视口参数
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Section1 准备顶点数据和FBO
	prepareVBO();
	// Section2 加载字形
	FontResourceManager::getInstance().loadFont("saveWater", "../../resources/fonts/saveWater.ttf");
	FontResourceManager::getInstance().loadASCIIChar("saveWater", 38, unicodeCharacters);
	FontResourceManager::getInstance().loadUnicodeText("saveWater", 38, std::wstring(L"关关雎鸠在河之洲窈窕淑女君子好逑"), unicodeCharacters);
	// Section3 准备着色器程序
	Shader shader("scene.vertex", "scene.frag");

	glEnable(GL_DEPTH_TEST);
	// 注意绘制文字时开启透明特性
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性
		
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 绘制文字时使用正交投影
		glm::mat4 projection = glm::ortho(0.0f, (GLfloat)(WINDOW_WIDTH),0.0f,(GLfloat)WINDOW_HEIGHT);
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵

		// 这里填写场景绘制代码
		shader.use();
		// 设置变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model;
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));

		GLfloat xOffset = 0, yOffset = 0;
		// xOffset = sin(glfwGetTime()) / 4.0  * WINDOW_WIDTH;
		// yOffset = cos(glfwGetTime()) / 4.0 * WINDOW_HEIGHT; // 偏移随机位置
		GLfloat xPos = WINDOW_WIDTH / 3.0f + xOffset;
		GLfloat yPos = WINDOW_HEIGHT / 2.0f + yOffset;

		renderText(shader, L"Hello World",
			xPos,
			yPos,
			1.4f, glm::vec3(1.0f, 1.0f, 0.0f));
		renderText(shader, L"关关雎鸠 在河之洲", 
			xPos + 35.0f,
			yPos - 55.0f,
			0.8f, glm::vec3(0.0f, 0.0f, 1.0f));
		renderText(shader, L"窈窕淑女 君子好逑",
			xPos + 35.0f,
			yPos - 95.0f,
			0.8f, glm::vec3(0.0f, 0.0f, 1.0f));
		renderText(shader, L"Render With OpenGL",
			xPos + 55.0f,
			yPos - 200.0f,
			0.8f, glm::vec3(0.0f, 0.0f, 1.0f));
		
		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &quadVAOId);
	glDeleteBuffers(1, &quadVBOId);
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

void prepareVBO()
{
	glGenVertexArrays(1, &quadVAOId);
	glGenBuffers(1, &quadVBOId);
	glBindVertexArray(quadVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
	// 绘制文字的矩形顶点属性数据 位置 纹理 是动态计算出来的
	// 这里预分配空间
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
    // xy 表示位置 zw表示纹理坐标
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
}

void renderText(Shader &shader, std::wstring text,
	GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	shader.use();
	glUniform3f(glGetUniformLocation(shader.programId, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(quadVAOId);

	// 遍历绘制字符串中字符
	std::wstring::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (unicodeCharacters.count(*c) <= 0)
		{
			continue;
		}
		FontCharacter ch = unicodeCharacters[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;
		// 计算这个字形对应的矩形位置数据
		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos, ypos, 0.0, 1.0 },
			{ xpos + w, ypos, 1.0, 1.0 },

			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos + w, ypos + h, 1.0, 0.0 }
		};
		// 绑定这个字符对应的纹理
		glBindTexture(GL_TEXTURE_2D, ch.textureId);
		// 动态设置VBO中数据 因为这个表示文字的矩形位置数据有变动 需要动态调整
		glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 因为advance 以1/64像素表示距离单位 所以这里移位后表示一个像素距离
		x += (ch.advance >> 6) * scale; 
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}