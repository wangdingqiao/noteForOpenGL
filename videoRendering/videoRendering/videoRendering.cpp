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
#include <cstdlib>
#include <ctime>

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
// 包含视频渲染辅助类
#include "videoObject.h"

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
GLuint PBOIds[2];
void setupQuadVAO();
void preparePBO();
// 图片参数
const int    IMAGE_WIDTH = WINDOW_WIDTH;
const int    IMAGE_HEIGHT = WINDOW_HEIGHT;
const int DATA_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 4;
const int    TEXT_WIDTH = 8;
const int    TEXT_HEIGHT = 30;
const GLenum PIXEL_FORMAT = GL_RGBA;
void updatePixels(GLubyte* dst, int size);
GLubyte* imageData = 0; // 图片数据
void initImageData();
void releaseImageData();

//视频渲染对象
VideoObject videoObject;

int pboMode = 2;
Timer timer, t1, t2;
float copyTime, updateTime;
GLuint textureId;
void initTexture();

void renderInfo(Shader& shader);
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
		"Demo of render video", NULL, NULL);
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
	initImageData();
	initTexture();
	std::srand(std::time(0));
	// Section2 加载字体
	FontResourceManager::getInstance().loadFont("arial", "../../resources/fonts/arial.ttf");
	FontResourceManager::getInstance().loadASCIIChar("arial", 38, unicodeCharacters);

	// Section3 加载视频
	const std::string fileName = "../../resources/videos/movie.wmv";
	if (!videoObject.openVideo(fileName, IMAGE_WIDTH, IMAGE_HEIGHT))
	{
		std::cerr << " failed to open video file: " << fileName << std::endl;
		char waitKey;
		std::cin >> waitKey;
		return -1;
	}
	// Section3 准备着色器程序
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

		glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 100.0f); // 投影矩阵
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵
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
		renderScene(shader);
		// 在绘制信息文字
// 		textShader.use();
// 		projection = glm::ortho(0.0f, (GLfloat)(WINDOW_WIDTH),
// 		0.0f, (GLfloat)WINDOW_HEIGHT);
// 		view = glm::mat4();
// 		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "projection"),
// 			1, GL_FALSE, glm::value_ptr(projection));
// 		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "view"),
// 			1, GL_FALSE, glm::value_ptr(view));
// 		model = glm::mat4();
// 		glUniformMatrix4fv(glGetUniformLocation(textShader.programId, "model"),
// 			1, GL_FALSE, glm::value_ptr(model));
// 		renderInfo(textShader);

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
	releaseImageData();

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
		pboMode = (pboMode + 1) % 3;
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
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOIds[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOIds[1]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// 写入视频到纹理中去
void updatePixels(GLubyte* dst, int size)
{
	if (!dst)
		return;
	if (!videoObject.getNextFrame(dst, size))
	{
		std::cerr << "Error failed to get next frame" << std::endl;
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

void renderScene(Shader& shader)
{
	static int index = 0;				// 用于从PBO拷贝像素到纹理对象
	int nextIndex = 0;                  // 指向下一个PBO 用于更新PBO中像素
	glActiveTexture(GL_TEXTURE0);
	if (pboMode > 0)
	{
		if (pboMode == 1)
		{
			// 只有一个时 使用0号PBO
			index = nextIndex = 0;
		}
		else if (pboMode == 2)
		{
			index = (index + 1) % 2;
			nextIndex = (index + 1) % 2;
		}

		// 开始PBO到texture object的数据复制 unpack操作
		t1.start();

		// 绑定纹理 和PBO
		glBindTexture(GL_TEXTURE_2D, textureId);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOIds[index]);

		// 从PBO复制到texture object 使用偏移量 而不是指针
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH,
			IMAGE_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

		// 计算复制数据所需时间
		t1.stop();
		copyTime = t1.getElapsedTimeInMilliSec();


		// 开始修改nextIndex指向的PBO的数据
		t1.start();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOIds[nextIndex]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
		// 从PBO映射到用户内存空间 然后修改PBO中数据
		GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (ptr)
		{
			// 更新映射后的内存数据
			updatePixels(ptr, DATA_SIZE);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // 释放映射的用户内存空间
		}

		// 计算修改PBO数据所需时间
		t1.stop();
		updateTime = t1.getElapsedTimeInMilliSec();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	else
	{
		// 不使用PBO的方式 从用户内存复制到texture object
		t1.start();

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
			IMAGE_WIDTH, IMAGE_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)imageData);

		t1.stop();
		copyTime = t1.getElapsedTimeInMilliSec();

		// 修改内存数据
		t1.start();
		updatePixels(imageData, DATA_SIZE);
		t1.stop();
		updateTime = t1.getElapsedTimeInMilliSec();
	}
	glUniform1i(glGetUniformLocation(shader.programId, "randomText"), 0);
	
	// 绘制多个矩形显示纹理
	glBindVertexArray(quadVAOId);
	glm::vec2  positions[] = {
		glm::vec2(-8, 8),
		glm::vec2(0.0, 8),
		glm::vec2(8, 8),
		
		glm::vec2(-8, 2),
		glm::vec2(0.0, 2),
		glm::vec2(8, 2),

		glm::vec2(-8, 0.0f),
		glm::vec2(0.0, 0.0f),
		glm::vec2(8, 0.0f),

		glm::vec2(-8, -2),
		glm::vec2(0.0, -2),
		glm::vec2(8, -2),

		glm::vec2(-8, -8),
		glm::vec2(0.0, -8),
		glm::vec2(8, -8),
	};
	const double FAR_Z_POS = -30.0f;
	static double zPos[15] = { FAR_Z_POS, FAR_Z_POS, FAR_Z_POS, 
		FAR_Z_POS, FAR_Z_POS, FAR_Z_POS ,
		FAR_Z_POS, FAR_Z_POS, FAR_Z_POS };
	for (size_t i = 0; i < sizeof(positions) / sizeof(positions[0]); ++i)
	{
		glm::mat4 model = glm::mat4();
		zPos[i] += (std::rand() % 10) * 0.1f;
		if (zPos[i] - 0.1f >= 0.0001f)
		{
			zPos[i] = FAR_Z_POS;
		}
		glm::vec3 pos = glm::vec3(positions[i].x, positions[i].y, zPos[i]);
		model = glm::translate(model, pos);
		model = glm::scale(model, glm::vec3(1.4));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindVertexArray(0);
}
void renderInfo(Shader& shader)
{
	float color[4] = { 1, 1, 1, 1 };

	std::wstringstream ss;
	ss << "PBO: ";
	if (pboMode == 0)
		ss << "off" << std::ends;
	else if (pboMode == 1)
		ss << "1 PBO" << std::ends;
	else if (pboMode == 2)
		ss << "2 PBOs" << std::ends;

	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - TEXT_HEIGHT, 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << std::fixed << std::setprecision(3);
	ss << "Updating Time: " << updateTime << " ms" << std::ends;
	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - (2 * TEXT_HEIGHT), 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << "Copying Time: " << copyTime << " ms" << std::ends;
	renderText(shader, ss.str(), 1, WINDOW_HEIGHT - (3 * TEXT_HEIGHT), 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
	ss.str(L"");

	ss << "Press SPACE key to toggle PBO on/off." << std::ends;
	renderText(shader, ss.str(), 1, 1, 0.8f, glm::vec3(0.0f, 0.0f, 1.0f));

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
void initImageData()
{
	imageData = new GLubyte[DATA_SIZE];
	memset(imageData, 0, DATA_SIZE);
}
void initTexture()
{
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, IMAGE_WIDTH, IMAGE_HEIGHT, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)imageData);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void releaseImageData()
{
	delete[] imageData;
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