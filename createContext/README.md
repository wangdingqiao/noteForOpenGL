写在前面
  如果你正在阅读本文，那么你和我一样可能喜欢图形学或者游戏编程，想尝试编写自己的图形App :)。图形学的初学者往往非常着急，希望能立即编写出丰富生动、逼真又富有交互性的3D应用，至少我是这样。
	
  笔者翻看了图形学教材，红宝书，3D数学基础等大部头书后，感觉到根据教材或者网络tutorial编写简单的应用程序并不困难，难点在于能够保持足够热情，坚持学习，使自己对图形学原理有更深更完整理解，培养自主学习更高级话题的能力和解决实际问题的能力。
 贪多嚼不烂，如果过于急躁，学习教材或者网络tutorial就会感觉要么太简单，要么太难，难以持之以恒。因此我写下这一系列博客，目的在于保持自己的热情，保持自己的节凑，以OpenGL编程为切入点，扎实学习图形学原理，从而增强图形编程能力。
 
   在学习OpenGL编程的过程中，当前感触比较深的是:**宁愿暂时跳过一些复杂的数学背景知识，也不要因此损害到热情，兴趣非常重要。**因此和我一样对图形学有热情的同路人，如果遇到难点一定不要轻易放弃，如果一次不能完全理解，可以标记下来，回过头来再做研究。
   本系列将使用 windows & visual studio 2013 &  C++ 作为开发平台，笔者建议初学者也是如此，尽量不要把精力耗费在linux下库文件编译和显卡程序安装，编程语言细节（例如C++11新特性，java语言绑定版本）等琐碎事情上，更多地关注图形原理和实现。
   
 话不絮烦，下面开始本节内容。
 
通过本节，可以了解到:

 - OpenGL基本特点
 - Windows下OpenGL开发环境搭建
 -  第一个OpenGL程序-准备Context
 
##OpenGL基本特点
   OpenGL是一个跨语言和跨平台的图形编程接口，这个API用来与GPU交互，实现基于硬件加速的图形渲染。OpenGL实际上是一个抽象的图形绘制规范，尽管可以由软件来完全实现，但它被设计为大部分或者全部由硬件来实现。OpenGL是一个绘图规范，不同的显卡厂商负责具体实现，因此实现的具体程序也不尽相同。OpenGL是跨平台的，支持PC端，嵌入式设备，手机设备的渲染；同时也是跨语言的，它底层使用C语言实现，支持java语言、Python、javaScript绑定。
     OpenGL包括旧的版本和现代版本  旧版本的绘图功能大多数在GPU内完成，某些功能无法实现，现代版本使用着色器程序提供了更大的灵活性。旧式版本是指1.x和2.x版本，现代版本是指3.x以上版本。两者函数的一个区别如下:
     
- 旧版函数: glBegin, glVertex3f, glLightfv, glPushMatrix
- 现代版函数：glDrawArrays, glVertexAttribPointer, glUniform, glCreateShader.

当3.2规范发布后，OpenGL支持两种profile:

-  Core Profile, 仅支持未被废弃特性。
-  The Compatibility Profile (兼容模式) 支持所有特性
尽管在兼容模式下，仍然可以运行那些已经被废弃的API编写的程序，但为了程序以后的兼容性，我们将使用OpenGL3.3版本，开启core profile。
想了解更多，关于为什么不要使用旧版OpenGL,可以参考[Don't use old OpenGL](https://kos.gd/posts/dont-use-old-opengl/),以及[Learning Modern OpenGL](http://www.codeproject.com/Articles/771225/Learning-Modern-OpenGL)。关于如何从旧版转移到现代版本，可以参考[Transition to Core Profile](http://retokoradi.com/2014/03/30/opengl-transition-to-core-profile/).

一个值得注意的问题是，为了保持跨平台，OpenGL规范中并未涉及到OpenGL context的创建和管理部分，这部分的实现依赖于不同系统的窗口管理系统。因此在编写OpenGL程序时需要借助第三方库完成与窗口系统的交互，创建绘制上下文对象(render context)。

## 开发环境搭建
上面已经提到OpenGL的具体实现由显卡驱动厂商负责，那么不同的厂商实现的库可能有所不同，有的函数可能没有实现，有的包含拓展的实现，而且厂商的实现一般通过动态链接库来提供。一般地使用一个函数前来查询函数是否实现了，而且要关心版本问题，这些细节比较繁琐，可以交给第三方库GLEW来实现。
同时与操作系统的窗口系统交互，创建绘制上下文对象，也是与具体操作系统相关的，可以交给第三方库GLFW来实现（当然还有其他选择，包括[ freeglut](http://freeglut.sourceforge.net/),[SDL](https://www.libsdl.org/),GLFW目前是比较推荐的）。
总结为我们需要下载和编译两个第三方库:

- [GLEW](https://www.opengl.org/sdk/libs/GLEW/) 负责绘图函数的获取
- [GLFW]( http://www.glfw.org/) 负责创建绘图上下文和窗口系统管理

下载并编译后，获取到两个库的lib和dll文件，我们就可以开始编写第一个程序了。编译源文件的过程，可以选择对应的visual studio版本，然后编译为默认的调试版本即可。如果觉得下载和编译源码费劲，我也为你提供了Windows下编译好的库文件和必要的头文件，[点击从github下载库文件](https://github.com/wangdingqiao/noteForOpenGL/tree/master/libraries)。

## 第一个OpenGL程序-准备Context
利用上面编译的两个第三方库，我们可以实现自己的第一个程序，创建用于渲染图形的上下文对象。
一般而言，OpenGL程序的流程大致都是这样（伪代码描述来自[open.gl](https://open.gl/context)）:
```C++
#include <libraryheaders>

int main()
{
    initLibraries();   // 初始化相关库
    createWindow(title, width, height); // 创建窗口
    createOpenGLContext(settings); // 创建绘制上下文

    while (windowOpen)  // 游戏主循环
    {
        while (event = newEvent())
            handleEvent(event); // 处理事件

        updateScene(); //更新场景中内容

        drawGraphics(); // 绘制场景
        presentGraphics(); // 交换缓冲区 呈现场景
    }
    freeResource();  // 资源清理
    return 0;
}
```
根据上述思路，我们开始创建本节的CreateContext程序。

**Step1 **: 新建项目CreateContext  在项目属性中链接头文件和库文件。在visual studio里面设置即可。
如下图 设置头文件目录和库文件目录：
![这里写图片描述](http://img.blog.csdn.net/20160503222754058)

如下图 链接库文件:
![这里写图片描述](http://img.blog.csdn.net/20160503222829430)

**Step2** 添加createContext.cpp文件。
首先我们要包含头文件如下:
``` C++
// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
```
这里注意两点:

-  首先包含glew,因为glew里面会包含gl.h等文件 这些文件是系统中OpenGL库文件
- 定义GLEW_STATIC表示静态链接glew库，当然也可以使用GLEW的动态链接库dll

包含头文件后，注意我们需要首先创建绘制上下文，然后才能初始化glew库([想了解更多请参考Initializing GLEW](http://glew.sourceforge.net/basic.html))。另外注意GLFW和GLEW初始化的返回值，对于出错情况要做好处理。
在上文提到，我们应该使用OpenGL core profile, 通过glfw的函数这样实现:
``` C++
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```
这里提示glfw开启OpenGL 3.3版本.
为了保持程序简单，不涉及放大缩小等问题，这里通过
```C++
     glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
```
使窗口固定大小。同时在绘制场景前，使用
```C++
     glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
```
为窗口指定显示区域大小，关于这个参数，我们后面会继续深入了解。

最后是窗口事件处理，这里以键盘事件为例，当用户按下Esc键时，退出应用程序，键盘事件在主循环中由glfwPollEvents处理，这个函数处理时会调用键盘事件的回调函数，因此我们需要声明和编写键盘回调函数如下:
``` C++
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
}
```
想了解更多的输入事件和参数详细解释，可以参考[glfw Input guide](http://www.glfw.org/docs/latest/input.html#input_key).
一个好的习惯时，程序主循环结束时，释放程序占用的资源，这里我们通过:
``` C++
glfwTerminate(); // 释放资源
```
释放GLFW分配的资源。当程序中分配有其他资源时，也需要做相应的回收处理， 这点要注意。
通过上述分析，完整的代码如下([点击从github下载代码](https://github.com/wangdingqiao/noteForOpenGL/tree/master/createContext)):
``` C++
// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
#include <iostream>


// 键盘回调函数原型声明
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// 定义程序常量
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;


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
		"Demo of createContext", NULL, NULL);
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

	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // 处理例如鼠标 键盘等事件

		// 清除颜色缓冲区 重置为指定颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 这里填写场景绘制代码 本节不绘制物体

		glfwSwapBuffers(window); // 交换缓存
	}
	glfwTerminate(); // 释放资源
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
}
```
 运行程序，效果如下:
 ![这里写图片描述](http://img.blog.csdn.net/20160503230426289)  

这里我们没有绘制任何内容，在[下一节](http://blog.csdn.net/wangdingqiaoit/article/details/51318793)将会绘制我们的第一个图形。

## 使用模板快速获取本节工程
有网友留言索要整个工程，因为github上面上传二进制的VS工程不太合适，这里制作了一个方便的模板供使用，可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/getting-started-template)。模板使用方法：

**Step1**: 将模板getting-started.zip拷贝值VS的项目模板目录，如下图所示：
![安装模板](http://img.blog.csdn.net/20160709103324246)


**Step2**: 使用模板新建工程，如下图：

![新建工程](http://img.blog.csdn.net/20160709103402880)

**Step3**: 将libraries拷贝至新建项目的同级目录下。
![拷贝libraries](http://img.blog.csdn.net/20160709103245051)

**Step4**: 编译运行新建工程即可。

##推荐阅读:
[1]: [OpenGL wikipedia](https://en.wikipedia.org/wiki/OpenGL)
[2]: [Don't use old OpenGL](https://kos.gd/posts/dont-use-old-opengl/)
[3]: [Learning Modern OpenGL](http://www.codeproject.com/Articles/771225/Learning-Modern-OpenGL)
[4]: [NVIDIA opengl](https://developer.nvidia.com/opengl)
## 参考资料
[1] Creating a window in [http://www.learnopengl.com/](http://www.learnopengl.com/#!Getting-started/Creating-a-window)
[2] Window and OpenGL context in [open.gl](https://open.gl/context)