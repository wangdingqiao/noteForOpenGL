Github公式无法渲染，请到[我的博客](http://blog.csdn.net/wangdingqiaoit/article/details/51586007)查看全文。
写在前面
   上一节[视变换(view transformation) ](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)，介绍了相机的设置参数，并建立了圆形坐标系和球形坐标系下的相机位置随着时间改变的绘制立方体程序。程序中用户无法通过键盘和鼠标来和场景中物体交互，本节实现一个第一人称相机来更好地与场景中物体交互。本节代码可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/FPS-Euler)。

通过本节可以了解到

- 欧拉角
- 第一人称相机的实现


## 欧拉角和相机
在[上一节](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)中，我们设置相机通过三个参数，分别为相机位置eye、相机指向目标位置target，以及viewUp向量。我们的目标是建立这样一个相机系统：通过键盘AD键在场景中左右移动，通过WS键在场景中前后移动，通过鼠标上下移动实现观察者向上和向下看的效果，通过鼠标左右移动实现观察者xoz水平面上的观察方向改变，还可以实现绕z轴的旋转来调节相机。这个过程如下图所示：

![相机控制](http://img.blog.csdn.net/20141009203911171?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvd2FuZ2RpbmdxaWFvaXQ=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

称绕着x轴的旋转为pitch角度，绕着y轴的旋转为yaw角度，称绕着z轴的旋转为roll角度。一般地实现第一人称相机只需要考虑pitch和yaw角度即可，roll角度用于飞行器的相机模型。
通过旋转pitch、yaw、roll角度，并将相机移到指定位置eye，那么对应的视变换矩阵为:

$view = (T*R_{roll}*R_{yaw}*R_{pitch})^{-1}$

要实现第一人称相机需要考虑的因素包括：

- 如何获取pitch和yaw旋转角度 ？
- 如何更新相机位置？
- 场景中移动速度如何决定？
- 如何获取视变换矩阵？
- 如何实现缩放 ？


## 获取pitch和yaw旋转角度
通过鼠标的水平位移来反映yaw角度的增长，垂直位移来反映pitch角度的增长，注册鼠标移动回调函数，代码如下：
```C++
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
```
程序初始化时，记录第一次移动位置为lastX和lastY，从第二次移动时开始处理，这样可以避免窗口刚出现时，xoffset和yoffset过大而引起相机抖动。
获取了鼠标位移，我们利用这个位移来建立与pitch和yaw联系。鼠标移动时，可以提供灵敏度供用户选择，这里我们使用默认值，代码如下：
```C++
   // 处理鼠标移动
void handleMouseMove(GLfloat xoffset, GLfloat yoffset)
{
	xoffset *= this->mouse_sensitivity; // 用鼠标灵敏度调节角度变换
	yoffset *= this->mouse_sensitivity;

	this->pitchAngle += yoffset;
	this->yawAngle += xoffset;
	// 保证角度在合理范围内
	if (this->pitchAngle > MAX_PITCH_ANGLE)
		this->pitchAngle = MAX_PITCH_ANGLE;
	if (this->pitchAngle < -MAX_PITCH_ANGLE)
	     this->pitchAngle = -MAX_PITCH_ANGLE;
	if (this->yawAngle < 0.0f)
	     this->yawAngle += 360.0f;
	this->updateCameraVectors(); // 更新相机向量
}
```
为了避免出现[万向锁](https://en.wikipedia.org/wiki/Gimbal_lock)，控制pitch在[-89.0,89.0]的范围内，控制yaw在[0,360.0]范围内。
## 重新计算forward等向量
对相机进行pitch和yaw角度的旋转后，我们需要重新计算相机的forward向量，以及side向量用来完成相机的前后左右移动。这两个向量都是在世界坐标系下给定的。通过$(R_{yaw}*R_{pitch})$我们可以计算出相机的坐标系下的点经过旋转后，在世界坐标系下的值。计算得到:
$$\begin{align}
R &= R_{yaw}R_{pitch} \\
& = \begin{bmatrix}
cos_{yaw} & 0 & sin_{yaw} & 0 \\
0 & 1 & 0 & 0 \\
-sin_{yaw} & 0 & cos_{yaw} & 0 \\
0 & 0 & 0 & 1
\end{bmatrix} *
\begin{bmatrix}
1 & 0 & 0 & 0 \\
0 & cos_{pitch} & -sin_{pitch} &  0 \\
0 & sin_{pitch} & cos_{pitch} & 0 \\
0 & 0 & 0 & 1
\end{bmatrix} \\
&= \begin{bmatrix} 
cos_{yaw} & sin_{yaw}sin_{pitch} & sin_{yaw}cos_{pitch} & 0 \\
0 & cos_{pitch} & -sin_{pitch} & 0 \\
-sin_{yaw} & cos_{yaw}sin_{pitch} & cos_{yaw}cos_{pitch} & 0 \\
0 & 0 & 0 & 1
\end{bmatrix} 
\end{align}$$
通过矩阵R可以计算得到原始的forward=(0,0,-1,0)向量变换后的向量，计算结果为上述矩阵R第三列求反的结果，表示为：
```C++
glm::vec3 forward;
forward.x = -sin(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
forward.y = sin(glm::radians(this->pitchAngle));
forward.z = -cos(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
this->forward = glm::normalize(forward);
```
定义初始的pitch = 0.0f, yaw = 0.0f，则上面的式子计算得到(0,0,-1)正好是初始的forward向量。
同时我们通过AS键在场景中水平移动时，需要重新计算side向量，side初始为(1,0,0)，旋转后的结果为上述矩阵R的第一列，计算过程表示为：
```C++
glm::vec3 side;
side.x = cos(glm::radians(this->yawAngle));
side.y = 0;
side.z = -sin(glm::radians(this->yawAngle));
this->side = glm::normalize(side);
```

## 动态更新相机位置
相机位置计算为$eye + forward$向量，我们通过ASWD来在场景中移动时，就是通过移动相机位置来实现，在键盘回调函数中监听按键状态，根据按键的状态来移动相机位置。按键状态，通过bool数组实现，代码为：
```C++
 bool keyPressedStatus[1024]; // 按键情况记录
 void key_callback(GLFWwindow* window, int key, 
 int scancode, int action, int mods)
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
```
根据按键状态，移动相机位置。定义移动方向的枚举变量：
```C++
// 定义移动方向
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};
```
在主循环中，根据按键状态调用相机类的处理函数：
```C++
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
```
相机类中计算相机位置的实现为:
```C++
void handleKeyPress(Camera_Movement direction, 
GLfloat deltaTime)
	{
		GLfloat velocity = this->moveSpeed * deltaTime;
		switch (direction)
		{
		case FORWARD:
			this->position += this->forward * velocity;
			break;
		case BACKWARD:
			this->position -= this->forward * velocity;
			break;
		case LEFT:
			this->position -= this->side * velocity;
			break;
		case RIGHT:
			this->position += this->side * velocity;
			break;
		default:
			break;
		}
	}
```
上述实现中，我们通过定义moveSpeed和deltaTime来表示移动距离。deltaTime是绘制两帧的间隔，不同计算机绘制帧的速度不一样，通过deltaTime可以平衡不同机器之间速度的差异，从而达到移动速度与机器处理速度无关的效果。deltaTime计算如下：
```C++
// 开始游戏主循环
while (!glfwWindowShouldClose(window))
{
	GLfloat currentFrame = (GLfloat)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	...
}
```
## 计算视变换矩阵
通过上面计算出了相机的新的位置，以及新的forward向量，我们可以通过[上一节](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)介绍的glm::lookAt函数来获取视变换矩阵如下：
```C++
// 获取视变换矩阵
glm::mat4 getViewMatrix()
{
   return glm::lookAt(this->position, 
	this->position + this->forward, this->viewUp);
}
```
其中viewUp初始值为(0,1,0)，当相机进行旋转和移动时，保持viewUp不变。
## 实现缩放
缩放时通过调节视角(Fov)来实现的，这个是投影变换中的概念，这里给出一个实现，后面会对投影变换进行单独介绍。调节Fov来改变视角，这个由鼠标的滚轮来实现，鼠标滚轮的回调函数实现为：
```C++
 // 由相机辅助类处理鼠标滚轮控制
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.handleMouseScroll(yoffset);
}
// 处理鼠标滚轮缩放 保持在[1.0, 45.0]之间
void handleMouseScroll(GLfloat yoffset)
{
	if (this->mouse_zoom >= 1.0f 
	&& this->mouse_zoom <= MOUSE_ZOOM)
		this->mouse_zoom -= this->mouse_sensitivity * yoffset;
	if (this->mouse_zoom <= 1.0f)
		this->mouse_zoom = 1.0f;
	if (this->mouse_zoom >= 45.0f)
		this->mouse_zoom = 45.0f;
}
```
设置投影变换矩阵为：
```C++
   // 投影矩阵
glm::mat4 projection = glm::perspective(camera.mouse_zoom,
	(GLfloat)(WINDOW_WIDTH)/ WINDOW_HEIGHT, 1.0f, 100.0f);
```

## 键盘鼠标对应的回调函数和设置
实现第一人称相机键盘和鼠标回调函数，需要注册的回调函数包括：
```C++
// 注册窗口键盘事件回调函数
glfwSetKeyCallback(window, key_callback);
// 注册鼠标事件回调函数
glfwSetCursorPosCallback(window, mouse_move_callback);
// 注册鼠标滚轮事件回调函数
glfwSetScrollCallback(window, mouse_scroll_callback);
```
另外，FPS相机需要隐藏鼠标光标，并将鼠标限制在窗口内，捕获鼠标动作。利用GLFW的设置选项实现为：
```C++
// 捕获鼠标
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
```

## 使用第一人称相机观察立方体
使用[上一节](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)绘制立方体的程序，将上述相机实现写到一个相机类Camera里面，在程序中使用相机类观察立方体的效果如下：
![场景中移动](http://img.blog.csdn.net/20160604210135834)

## 最后的说明
本节实现的第一人称相机，使用的是欧拉角的方法。欧拉角实现过程中旋转顺序，旋转角度的定义，通常很自由，这个导致计算视变换矩阵时，网络上的教程中公式也不尽相同。另外一种实现方法是，使用球面坐标系统，通过指定pitch、yaw角度，以及定义球的半径为1来实现。可以参考[http://www.learnopengl.com](http://www.learnopengl.com/#!Getting-started/Camera)中旋转后forward的计算过程。另外，本节实现的相机，无法避免万向锁。更好地方式是使用四元素来实现。关于[万向锁](https://en.wikipedia.org/wiki/Gimbal_lock)、[四元素](http://www.3dgep.com/understanding-quaternions/)，这个主题稍微复杂一些，后面会专门作为一节学习。

## 参考资料
1.[Camera  on www.learnopengl.com](http://www.learnopengl.com/#!Getting-started/Camera)
2.[A C++ Camera Class for Simple OpenGL FPS Controls](http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/)
3.[Tutorial 6 : Keyboard and Mouse](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/)
4.[Modern OpenGL 04 - Cameras, Vectors & Input](http://www.tomdalling.com/blog/modern-opengl/04-cameras-vectors-and-input/)
