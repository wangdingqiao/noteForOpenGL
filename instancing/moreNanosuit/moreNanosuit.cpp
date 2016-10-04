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

void prepareInstanceMatrices(std::vector<glm::mat4>& modelMatrices, 
	const int amount, const Model& instanceModel);

// 定义程序常量
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
// 用于相机交互参数
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouseMove = true;
bool keyPressedStatus[1024]; // 按键情况记录
GLfloat deltaTime = 0.0f; // 当前帧和上一帧的时间差
GLfloat lastFrame = 0.0f; // 上一帧时间
const int INSTANCE_COUNT = 441;  // 填写可以开平方的奇数
const GLfloat DELTA_ROW = 15.0f, DELTA_COL = 10.0f;
const int ROWS = (int)(sqrt(INSTANCE_COUNT)), COLS = (int)(sqrt(INSTANCE_COUNT));
const GLfloat X_LEN = DELTA_ROW * (ROWS - 1), Z_LEN = DELTA_COL * (COLS - 1); // 平面x z 方向长度
Camera camera(glm::vec3(X_LEN / 5.0f, 14.0f, Z_LEN / 2.0f + 25.0f));

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
		"Demo of instancing model", NULL, NULL);
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
	
	//Section1 加载模型数据 为了方便更换模型 我们从文件读取模型文件路径
	Model nanosuit;
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
	if (modelFilePath.empty())
	{
		std::cerr << "Error::model path empty." << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	if (!nanosuit.loadModel(modelFilePath))
	{
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	// section2 指定地面顶点数据和加载纹理
	// 地板顶点属性数据 顶点位置 纹理坐标(设置的值大于1.0用于重复)
	GLfloat planeVertices[] = {
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,   // A
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f,  // D
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f, // C

		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f, // C
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,  // B
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,   // A
	};
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
	GLuint planeTextId = TextureHelper::load2DTexture("../../resources/textures/metal.png");
	// Section2 准备着色器程序
	Shader shader("plane.vertex", "plane.frag");
	Shader instanceShader("instance.vertex", "instance.frag");

	// Section3 为多个实例准备模型变换矩阵
	std::vector<glm::mat4> modelMatrices;
	prepareInstanceMatrices(modelMatrices, INSTANCE_COUNT, nanosuit);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		std::cout << " time used in this frame: " << deltaTime << " seconds." << std::endl;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性

		// 清除颜色缓冲区 重置为指定颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 1000.0f); // 投影矩阵
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵
		// 这里填写场景绘制代码
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(planeVAOId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTextId);
		glm::mat4 model = glm::mat4();
		model = glm::scale(model, glm::vec3(X_LEN / 5.0f, 1.0, Z_LEN / 5.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// 绘制多个纳米战斗服人
		instanceShader.use();
		glUniformMatrix4fv(glGetUniformLocation(instanceShader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(instanceShader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		const std::vector<Mesh>& meshes = nanosuit.getMeshes();
		for (std::vector<Mesh>::size_type i = 0; i < meshes.size(); ++i)
		{
			glBindVertexArray(meshes[i].getVAOId());
			const int texUnitCnt = meshes[i].bindTextures(instanceShader);
			glDrawElementsInstanced(GL_TRIANGLES, 
				meshes[i].getIndices().size(), GL_UNSIGNED_INT, 0, modelMatrices.size());
			glBindVertexArray(0);
			meshes[i].unBindTextures(texUnitCnt);
		}

		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
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
// 为实例准备模型变换矩阵
// 这里的细节可以不用深究
void prepareInstanceMatrices(std::vector<glm::mat4>& modelMatrices, 
	const int amount, const Model& instanceModel)
{
	for (GLint row = -ROWS / 2; row <= ROWS / 2; ++row)
	{
		for (GLint col = - COLS / 2; col <= COLS / 2; ++col)
		{ 
			glm::vec3 translation;
			translation.x = -(X_LEN / 2.0) + (col + COLS / 2) * DELTA_ROW;
			translation.y = -0.75f;
			translation.z = -(Z_LEN / 2.0) + (row + ROWS / 2) * DELTA_COL;
			glm::mat4 model;
			model = glm::translate(model, translation);
			modelMatrices.push_back(model);
		}
	}
	// 创建instance array
	GLuint modelMatricesVBOId;
	glGenBuffers(1, &modelMatricesVBOId);
	glBindBuffer(GL_ARRAY_BUFFER, modelMatricesVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * modelMatrices.size(), 
		&modelMatrices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// 为模型里每个mesh 传递model matrix
	// 用4个vec4传递这个mat4类型
	const std::vector<Mesh>& meshes = instanceModel.getMeshes();
	for (std::vector<Mesh>::size_type i = 0; i < meshes.size(); ++i)
	{
		glBindVertexArray(meshes[i].getVAOId());
		glBindBuffer(GL_ARRAY_BUFFER, modelMatricesVBOId);
		// 第一列
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 
			4 * sizeof(glm::vec4), (GLvoid*)0);
		// 第二列
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 
			4 * sizeof(glm::vec4), (GLvoid*)(sizeof(glm::vec4)));
		// 第三列
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 
			4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
		// 第四列
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
			4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));

		// 注意这里需要设置实例数据更新选项 指定1表示 每个实例更新一次
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}