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
#include <sstream>
#include <vector>

// 包含着色器加载库
#include "shader.h"
// 包含相机控制辅助类
#include "camera.h"
// 包含纹理加载辅助类
#include "texture.h"
// 计时器辅助类
#include "Timer.h"
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
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lampPos(0.5f, 0.5f, 0.3f);

GLuint quadVAOId, quadVBOId;
GLuint textVAOId, textVBOId;
GLuint cubeTextId, pixelTextId;
GLuint PBOIds[2];
void setupQuadVAO();
void preparePBO();
void initPixelText();
void drawRightSidePixel(Shader& shader);
void drawLeftSidePixel(Shader& shader);
// 图片参数
const int    SINGLE_SCREEN_WIDTH = WINDOW_WIDTH / 2;
const int    SINGLE_SCREEN_HEIGHT = WINDOW_HEIGHT;
const int	 DATA_SIZE = SINGLE_SCREEN_WIDTH * SINGLE_SCREEN_HEIGHT * 4;
const int    TEXT_WIDTH = 8;
const int    TEXT_HEIGHT = 30;
const GLenum PIXEL_FORMAT = GL_BGRA;
void updatePixels(unsigned char* src, int width, int height, int shift, unsigned char* dst);
GLubyte* colorBuffer = 0; // 像素数据
void initColorBuffer();
void releaseColorBuffer();

bool pboUsed = false;
Timer timer, t1;
float readTime, processTime;

void renderInfo(Shader& shader);
bool isFirstRendered = false;
void renderInitScene(Shader& shader);
void renderScene(Shader& shader);
void renderText(Shader &shader, std::wstring text,
	GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void printTransferRate();
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
		"Demo of PBO(asynchronous DMA transfer)", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 300, 100);
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

	// Section1 准备顶点数据
	setupQuadVAO();
	preparePBO();
	initColorBuffer();

	// Section2 加载字体
	FontResourceManager::getInstance().loadFont("arial", "../../resources/fonts/arial.ttf");
	FontResourceManager::getInstance().loadASCIIChar("arial", 38, unicodeCharacters);

	// Section3 加载纹理
	cubeTextId = TextureHelper::load2DTexture("../../resources/textures/wood.png");
	initPixelText();

	// Section4 准备着色器程序
	Shader shader("scene.vertex", "scene.frag");
	Shader textShader("text.vertex", "text.frag");

	glEnable(GL_DEPTH_TEST);
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

		// 清除颜色缓冲区 重置为指定颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;
		// 这里填写场景绘制代码
		// 先绘制纹理图片
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		if (!isFirstRendered)
		{
			renderInitScene(shader);
			isFirstRendered = true;
		}
		else
		{
			renderScene(shader);
		}
		// 在右侧绘制信息文字 否则左侧文字会复制到右侧
		textShader.use();
		projection = glm::ortho(0.0f, (GLfloat)(SINGLE_SCREEN_WIDTH),
		0.0f, (GLfloat)SINGLE_SCREEN_HEIGHT);
		view = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		renderInfo(textShader);
		printTransferRate();

		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &quadVAOId);
	glDeleteBuffers(1, &quadVBOId);
	glDeleteVertexArrays(1, &textVAOId);
	glDeleteBuffers(1, &textVBOId);
	glDeleteBuffers(2, PBOIds);  // 注意释放PBO
	releaseColorBuffer();

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
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		pboUsed = !pboUsed;
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

// 准备PBO
void preparePBO()
{
	glGenBuffers(2, PBOIds);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, PBOIds[0]);
	//  GL_STREAM_DRAW用于传递数据到texture object GL_STREAM_READ 用于读取FBO中像素
	glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, NULL, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, PBOIds[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, NULL, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

// 改变像素的亮度
void updatePixels(unsigned char* src, int width, int height, int shift, unsigned char* dst)
{
	if (!src || !dst)
		return;

	int value;
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			value = *src + shift;
			if (value > 255) *dst = (unsigned char)255;
			else            *dst = (unsigned char)value;
			++src;
			++dst;

			value = *src + shift;
			if (value > 255) *dst = (unsigned char)255;
			else            *dst = (unsigned char)value;
			++src;
			++dst;

			value = *src + shift;
			if (value > 255) *dst = (unsigned char)255;
			else            *dst = (unsigned char)value;
			++src;
			++dst;

			++src;    // 跳过alpha
			++dst;
		}
	}
}

// 建立矩形的VAO数据
void setupQuadVAO()
{
	// 顶点属性数据 位置 纹理
	GLfloat quadVertices[] = {
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &quadVAOId);
	glGenBuffers(1, &quadVBOId);
	glBindVertexArray(quadVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glGenVertexArrays(1, &textVAOId);
	glGenBuffers(1, &textVBOId);
	glBindVertexArray(textVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, textVBOId);
	// 绘制文字的矩形顶点属性数据 位置 纹理 是动态计算出来的
	// 这里预分配空间
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	// xy 表示位置 zw表示纹理坐标
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindVertexArray(0);
}

// 绘制一个初始场景 保证当FBO里使用glReadPixels时有内容
void renderInitScene(Shader& shader)
{
	// 绘制左边正方形
	drawLeftSidePixel(shader);
	// 绘制右边正方形
	drawRightSidePixel(shader);
}
void drawLeftSidePixel(Shader& shader)
{
	glViewport(0, 0, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTextId);
	glBindVertexArray(quadVAOId);
	glUniform1i(glGetUniformLocation(shader.programId, "cubeText"), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void drawRightSidePixel(Shader& shader)
{
	glViewport(SINGLE_SCREEN_WIDTH, 0, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT);
	// 使用着色器时 下面这种方式无效 使用一个纹理替代
	/*
	glDrawBuffer(GL_BACK);
	glRasterPos2i(0, 0);
	glDrawPixels(SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);
	*/
	glBindVertexArray(quadVAOId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pixelTextId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
		SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)colorBuffer);
	glUniform1i(glGetUniformLocation(shader.programId, "cubeText"), 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void renderScene(Shader& shader)
{
	static int shift = 0;
	static int index = 0;				// 用于从默认FBO读取到PBO
	int nextIndex = 0;                  // 用于从PBO更新像素 绘制到默认FBO
	shift = ++shift % 200;
	glReadBuffer(GL_FRONT); // 设置读取的FBO
	if (pboUsed)
	{
		index = (index + 1) % 2;
		nextIndex = (index + 1) % 2;
		// 开始FBO到PBO复制操作 pack操作
		t1.start();
		glBindBuffer(GL_PIXEL_PACK_BUFFER, PBOIds[index]);
		// OpenGL执行异步的DMA传输 这个命令会立即放回 此时CPU可以执行其他任务
		glReadPixels(0, 0, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

		// 计算读取数据所需时间
		t1.stop();
		readTime = t1.getElapsedTimeInMilliSec();


		// 开始修改nextIndex指向的PBO的数据
		t1.start();

		glBindBuffer(GL_PIXEL_PACK_BUFFER, PBOIds[nextIndex]);
		glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
		// 从PBO映射到用户内存空间 然后修改PBO中数据
		GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (ptr)
		{
			// 更新映射后的内存数据
			updatePixels(ptr, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, shift, colorBuffer);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // 释放映射的client memory
		}

		// 计算更新PBO数据所需时间
		t1.stop();
		processTime = t1.getElapsedTimeInMilliSec();
		// 注意 不要误写为glBindBuf(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	else
	{
		// 不使用PBO的方式 读取到client memory并修改
		t1.start();
		glReadPixels(0, 0, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);
		t1.stop();
		readTime = t1.getElapsedTimeInMilliSec();

		// 修改内存数据
		t1.start();
		updatePixels(colorBuffer, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, shift, colorBuffer);
		t1.stop();
		processTime = t1.getElapsedTimeInMilliSec();
	}
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawLeftSidePixel(shader);
	drawRightSidePixel(shader);
}

void renderInfo(Shader& shader)
{
	float color[4] = { 1, 1, 1, 1 };

	std::wstringstream ss;
	ss << "PBO: ";
	if (pboUsed)
		ss << "on" << std::ends;
	else
		ss << "off" << std::ends;

	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - TEXT_HEIGHT, 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << std::fixed << std::setprecision(3);
	ss << "Read Time: " << readTime << " ms" << std::ends;
	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - (2 * TEXT_HEIGHT), 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << "Process Time: " << processTime << " ms" << std::ends;
	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - (3 * TEXT_HEIGHT), 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << "Press SPACE key to toggle PBO on/off." << std::ends;
	renderText(shader, ss.str(), 1, 1, 0.6f, glm::vec3(0.0f, 0.0f, 1.0f));

	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
}
void printTransferRate()
{
	const double INV_MEGA = 1.0 / (1024 * 1024);
	static Timer timer;
	static int count = 0;
	static std::stringstream ss;
	double elapsedTime;

	// 循环直到1s时间到
	elapsedTime = timer.getElapsedTime();
	if (elapsedTime < 1.0)
	{
		++count;
	}
	else
	{
		std::cout << std::fixed << std::setprecision(1);
		std::cout << "Transfer Rate: " << (count / elapsedTime) * DATA_SIZE * INV_MEGA << " MB/s. (" << count / elapsedTime << " FPS)\n";
		std::cout << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
		count = 0;                      // 重置计数器
		timer.start();                  // 重新开始计时器
	}
}
void initColorBuffer()
{
	colorBuffer = new GLubyte[DATA_SIZE];
	memset(colorBuffer, 255, DATA_SIZE);
}
void releaseColorBuffer()
{
	delete[] colorBuffer;
}
void renderText(Shader &shader, std::wstring text,
	GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	shader.use();
	glUniform3f(glGetUniformLocation(shader.programId, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAOId);

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
		glBindBuffer(GL_ARRAY_BUFFER, textVBOId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 因为advance 以1/64像素表示距离单位 所以这里移位后表示一个像素距离
		x += (ch.advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initPixelText()
{
	glGenTextures(1, &pixelTextId);
	glBindTexture(GL_TEXTURE_2D, pixelTextId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SINGLE_SCREEN_WIDTH, SINGLE_SCREEN_HEIGHT, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)colorBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}
