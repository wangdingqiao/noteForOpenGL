/**
  如何手动计算LookAt矩阵
*/
// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
// 引入GLM库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>      // std::setprecision
#include <cstdlib>

void printMat4(const glm::mat4& mat);
void printVec4(const glm::vec4& srcVec4);
void testPrintFunc();
void testLookAt();
glm::mat4 computeLookAtMatrix1(glm::vec3 eye, glm::vec3 target, glm::vec3 viewUp);
glm::mat4 computeLookAtMatrix2(glm::vec3 eye, glm::vec3 target, glm::vec3 viewUp);

int main(int argc, char** argv)
{
	testLookAt();
	std::system("pause");
	return 0;
}
void testPrintFunc()
{
	glm::vec4 pos1(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 pos2(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 pos3(0.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 pos4(1.0f, 2.0f, 3.0f, 1.0f);
	glm::mat4 mat(pos1, pos2, pos3, pos4); // 以列向量构造mat4
	// 打印向量
	printVec4(pos1);
	std::cout << "----------" << std::endl;
	// 打印GLM计算的平移矩阵
	std::cout << "GLM::translate matrix" << std::endl;
	glm::mat4 transMat4 = glm::translate(glm::mat4(1.0), glm::vec3(1.0, 2.0, 3.0));
	printMat4(transMat4);
	std::cout << "----------" << std::endl;
	// 打印用4个列向量构造的平移矩阵
	std::cout << "manual translate matrix" << std::endl;
	printMat4(mat);
}
void testLookAt()
{
	glm::vec3 eye(0.0, 5.8, 2.0); // eye pos
	glm::vec3 center(0.0, 0.0, 6.5); // center
	glm::vec3 up(0.0, 1.2, 0.0); // up vector

	// 打印GLM 计算的lookAt矩阵
	glm::mat4 lookAtMat4 = glm::lookAt(eye, center, up);
	std::cout << "GLM::lookAt matrix:" << std::endl;
	printMat4(lookAtMat4);
	std::cout << "----------" << std::endl;
	// 打印手工计算的lookAt矩阵1
	glm::mat4 manualMat4 = computeLookAtMatrix1(eye, center, up);
	std::cout << "manual lookAt matrix1:" << std::endl;
	printMat4(manualMat4);
	// 打印手工计算的lookAt矩阵2
	manualMat4 = computeLookAtMatrix2(eye, center, up);
	std::cout << "manual lookAt matrix2:" << std::endl;
	printMat4(manualMat4);
}
// 手动构造LookAt矩阵 方式1
glm::mat4 computeLookAtMatrix1(glm::vec3 eye, glm::vec3 target, glm::vec3 viewUp)
{
	glm::vec3 f = glm::normalize(target - eye); // forward vector
	glm::vec3 s = glm::normalize(glm::cross(f, viewUp)); // side vector
	glm::vec3 u = glm::normalize(glm::cross(s, f)); // up vector
	glm::mat4 lookAtMat(
		glm::vec4(s.x, u.x, -f.x, 0.0), // 第一列
		glm::vec4(s.y, u.y, -f.y, 0.0), // 第二列
		glm::vec4(s.z, u.z, -f.z, 0.0), // 第三列
		glm::vec4(-glm::dot(s, eye),
		-glm::dot(u, eye), glm::dot(f, eye), 1.0)  // 第四列
		);
	return lookAtMat;
}
// 手动构造LookAt矩阵 方式2
glm::mat4 computeLookAtMatrix2(glm::vec3 eye, glm::vec3 target, glm::vec3 viewUp)
{
	glm::vec3 f = glm::normalize(target - eye); // forward vector
	glm::vec3 s = glm::normalize(glm::cross(f, viewUp)); // side vector
	glm::vec3 u = glm::normalize(glm::cross(s, f)); // up vector
	glm::mat4 rotate(
		glm::vec4(s.x, u.x, -f.x, 0.0), // 第一列
		glm::vec4(s.y, u.y, -f.y, 0.0), // 第二列
		glm::vec4(s.z, u.z, -f.z, 0.0), // 第三列
		glm::vec4(0.0, 0.0, 0.0, 1.0)  //  第四列
		);
	glm::mat4  translate;
	translate = glm::translate(translate, -eye);
	return rotate * translate;
}



void printVec4(const glm::vec4& srcVec4)
{
	std::cout << std::fixed;
	std::cout << glm::to_string(srcVec4) << std::endl;
}
void printMat4(const glm::mat4& srcMat4)
{
	std::cout << std::fixed;
	const float *pSource = (const float*)glm::value_ptr(srcMat4);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			float number = pSource[4 * j + i];
			std::cout.width(5);
			if (abs(number) < 0.0001f)
				std::cout << 0 << "\t";
			else
				std::cout << std::setprecision(2) << number << "\t";
		}
		std::cout << std::endl;
	}
}
