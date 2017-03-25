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
// 加载模型的类
#include "model.h"


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
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

GLint drawMode = 1;

GLuint CubeVAOId, CubeVBOId;
GLuint quadVAOId, quadVBOId;
void prepareVBO();

struct SGBuffer
{
	GLuint FBOId;		// 存储FBO
	GLuint gPosTextId;	// 存储位置信息
	GLuint gNormalTextId; // 存储法向量信息
	GLuint gAlbedoSpecTextId;// 存储漫反射颜色 和镜面光强度
};
SGBuffer gBuffer;

void renderPlane(Shader& shader);
void renderCubes(Shader& shader);
bool prepareGBuffer(SGBuffer& gBuffer);

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
		"Demo of Deferred shading(Press 1-5 to view)", NULL, NULL);
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
	
	if (!prepareGBuffer(gBuffer))
	{
		std::cerr << "prepare G-buffer failed." << std::endl;
		glfwTerminate();
		char waitKey;
		std::cin >> waitKey;
		return -1;
	}

	// Section2 加载模型
	// 为了方便更换模型 我们从文件读取模型文件路径
	std::ifstream modelPath("modelPath.txt");
	if (!modelPath)
	{
		std::cerr << "Error::could not read model path file." << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	std::string modelFilePath;
	std::getline(modelPath, modelFilePath);
	Model objModel;
	if (!objModel.loadModel(modelFilePath))
	{
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	
	//Model objModel(modelFilePath.c_str());
	std::vector<glm::vec3> objPositions; // 模型位置
	objPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

	// Section3 准备着色器程序
	Shader gBufferShader("gbuffer.vertex", "gbuffer.frag");
	Shader sceneShader("scene.vertex", "scene.frag");

	// Section4 准备光源数据
	struct LightAttr
	{
		LightAttr(glm::vec3 pos, glm::vec3 diff, glm::vec3 spec)
		{
			position = pos;
			diffuse = diff;
			specular = spec;
		}
		LightAttr(){}
		glm::vec3 position;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float linear;   // 一次衰减系数
		float quadratic; // 二次衰减系数
	};
	srand(13);
	std::vector<LightAttr> seqLightAttr;
	const GLuint NR_LIGHTS = 32;
	for (GLuint i = 0; i < NR_LIGHTS; i++)
	{
		LightAttr lightAttr;
		// 位置随机一个偏移量
		GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
		GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		lightAttr.position = glm::vec3(xPos, yPos, zPos);
		// 随机一个[0.5, 1.0)颜色
		GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5; 
		GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5;
		GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5;
		lightAttr.diffuse = glm::vec3(rColor, gColor, bColor);
		lightAttr.specular = glm::vec3(rColor / 5.0f , gColor / 5.0f, bColor / 5.0f);
		lightAttr.linear = 0.7;
		lightAttr.quadratic = 1.8;
		seqLightAttr.push_back(lightAttr);
	}

	glEnable(GL_DEPTH_TEST);
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性
		
		glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f); // 投影矩阵
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵

		// 这里填写场景绘制代码
		glm::mat4 model = glm::mat4();
		// 第一遍 绘制场景中所有物体到GBuffer geometry-pass
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.FBOId);
			// 清除颜色缓冲区 重置为指定颜色
			// 注意这里需要设置alpha成分为0.0 因为需要用到alpha成分作为镜面光强度
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gBufferShader.use();
			// 设置变换矩阵
			glUniformMatrix4fv(glGetUniformLocation(gBufferShader.programId, "projection"),
				1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(gBufferShader.programId, "view"),
				1, GL_FALSE, glm::value_ptr(view));
			// 渲染多个模型
			for (size_t i = 0; i < objPositions.size(); ++i)
			{
				glm::mat4 model = glm::mat4();
				model = glm::translate(model, objPositions[i]);
				model = glm::scale(model, glm::vec3(0.25f));
				glUniformMatrix4fv(glGetUniformLocation(gBufferShader.programId, "model"), 1, GL_FALSE, glm::value_ptr(model));
				objModel.draw(gBufferShader);
			}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 第二遍利用GBuffer 渲染光照 lighting-pass
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		sceneShader.use();
		// 设置光源属性
		for (size_t i = 0; i < seqLightAttr.size(); ++i)
		{
			std::stringstream strStream;
			strStream << "lights[" << i << "]";
			std::string indexStr = strStream.str();
			
			glUniform3fv(glGetUniformLocation(sceneShader.programId,
				(indexStr + ".diffuse").c_str()), 1, glm::value_ptr(seqLightAttr[i].diffuse));
			glUniform3fv(glGetUniformLocation(sceneShader.programId,
				(indexStr + ".specular").c_str()), 1, glm::value_ptr(seqLightAttr[i].specular));
			glUniform3fv(glGetUniformLocation(sceneShader.programId,
				(indexStr + ".position").c_str()), 1, glm::value_ptr(seqLightAttr[i].position));
			glUniform1f(glGetUniformLocation(sceneShader.programId,
				(indexStr + ".linear").c_str()), seqLightAttr[i].linear);
			glUniform1f(glGetUniformLocation(sceneShader.programId,
				(indexStr + ".quadratic").c_str()), seqLightAttr[i].quadratic);
		}
		// 设置观察者位置
		GLint viewPosLoc = glGetUniformLocation(sceneShader.programId, "viewPos");
		glUniform3f(viewPosLoc, camera.position.x, camera.position.y, camera.position.z);
		// 设置渲染效果模式
		glUniform1i(glGetUniformLocation(sceneShader.programId, "drawMode"), drawMode);
		// 渲染到一个矩形上
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gPosTextId);
		glUniform1i(glGetUniformLocation(sceneShader.programId, "gPostionText"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gNormalTextId);
		glUniform1i(glGetUniformLocation(sceneShader.programId, "gNormalText"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpecTextId);
		glUniform1i(glGetUniformLocation(sceneShader.programId, "gAlbedoSpecText"), 2);
		glBindVertexArray(quadVAOId);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

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
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		drawMode = 1;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		drawMode = 2;
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		drawMode = 3;
	}
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		drawMode = 4;
	}
	else if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		drawMode = 5;
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
	// 指定立方体顶点属性数据 顶点位置 法向量 纹理 
	GLfloat cubeVertices[] = {
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,	// B
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,	// C
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,	// C
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,	// D
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A


		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,	// E
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0, 1.0f, // H
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,	// G
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,	// G
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,	// F
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,	// E

		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// D
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0, 1.0f,   // H
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,	// E
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,	// E
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,	// A
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// D

		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // F
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // G
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // C
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // C
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,// B
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,// F

		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // G
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0, 1.0f, // H
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // D
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // D
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // C
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // G

		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // A
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // E
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // F
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // F
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // B
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // A
	};
	// 创建物体缓存对象
	glGenVertexArrays(1, &CubeVAOId);
	glBindVertexArray(CubeVAOId);
	glGenBuffers(1, &CubeVBOId);
	glBindBuffer(GL_ARRAY_BUFFER, CubeVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 指定用于展示的矩形顶点属性数据 位置 纹理
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
}
/*
* 创建G-buffer
* 包含存储位置的 RGB成分
* 存储漫反射光RGB 和镜面光强度的alpha成分
* 存储法向量的RGB
*/
bool prepareGBuffer(SGBuffer& gBuffer)
{
	glGenFramebuffers(1, &gBuffer.FBOId);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.FBOId);
	// 位置信息
	glGenTextures(1, &gBuffer.gPosTextId);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosTextId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.gPosTextId, 0);
	// 法向量信息
	glGenTextures(1, &gBuffer.gNormalTextId);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormalTextId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.gNormalTextId, 0);
	// 漫反射光以及镜面光强度
	glGenTextures(1, &gBuffer.gAlbedoSpecTextId);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpecTextId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.gAlbedoSpecTextId, 0);
	// MRT-告知OPenGL可渲染到多个目标缓冲区 
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// 添加深度rbo绑定
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - 检查FBO完整性
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}