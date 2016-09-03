GitHub公式无法渲染，阅读完整版请到[我的博客](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)。
写在前面
 OpenGL中的坐标处理过程包括模型变换、视变换、投影变换、视口变换等内容，这个主题的内容有些多，因此分节学习，主题将分为5节内容来学习。上一节[模型变换](http://blog.csdn.net/wangdingqiaoit/article/details/51531002)，本节学习模型变换的下一阶段——视变换。到目前位置，主要在2D下编写程序，学习了视变换后，我们可以看到3D应用的效果了。本节示例程序均可在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/viewTransformation)。

通过本节可以了解到

- 视变换的概念
- 索引绘制立方体
- LookAt矩阵的推导(**对数学不感兴趣，可以跳过**)
- 相机位置随时间改变的应用程序

## 坐标处理的全局过程(了解，另文详述)
OpenGL中的坐标处理包括模型变换、视变换、投影变换、视口变换等内容，具体过程如下图1所示:

![坐标处理过程](http://img.blog.csdn.net/20140918202127843)

每一个过程处理都有其原因，这些内容计划将会在不同节里分别介绍，最后再整体把握一遍。
**今天我们学习第二个阶段——视变换。**

## 并不存在真正的相机
OpenGL成像采用的是虚拟相机模型。在场景中你通过模型变换，将物体放在场景中不同位置后，最终哪些部分需要成像，显示在屏幕上，主要由视变换和后面要介绍的投影变换、视口变换等决定。

其中视变换阶段，通过假想的相机来处理矩阵计算能够方便处理。对于OpenGL来说并不存在真正的相机，所谓的相机坐标空间(camera space 或者eye space)只是为了方便处理，而引入的坐标空间。

在现实生活中，我们通过移动相机来拍照，而在OpenGL中我们通过以相反方式调整物体，让物体以适当方式呈现出来。例如，初始时，相机镜头指向-z轴，要观察-z轴上的一个立方体的右侧面，那么有两种方式：

1. 相机绕着+y轴，旋转+90度，此时相机镜头朝向立方体的右侧面，实现目的。注意这时立方体并没有转动。

2. 相机不动，让立方体绕着+y轴，旋转-90度，此时也能实现同样的目的。注意这时相机没有转动。完成这一旋转的矩阵记作$R_{y}(-\frac{\pi}{2})$

在OpenGL中，采用方式2来完成物体成像的调整。例如下面的图表示了假想的相机：

![相机](http://img.blog.csdn.net/20160602164650710)

------
###进一步说明
进一步说明这里相对的概念，对这个概念不感兴趣的可以跳过。默认时相机位于(0,0,0)，指向-z轴，相当于调用了：

```cpp
glm::lookAt(glm::vec(0.0f,0.0f,0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)),
```
得到是单位矩阵，这是相机的默认情况。

**上述第一种方式**，相机绕着+y轴旋转90度，相机指向-x轴，则等价于调用变为:

```cpp
   glm::mat4 view =glm::lookAt(glm::vec(0.0f,0.0f,0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)),
```
得到的视变换矩阵为:

$$ view =\begin{bmatrix} 0 & 0 & -1 & 0 \\
 0 & 1 & 0  & 0 \\
 1 & 0 & 0 & 0 \\
 0 & 0 & 0 & 1 
\end{bmatrix}$$

**上述第二种方式**，通过立方体绕着+y轴旋转-90度，则得到的矩阵M,相当于:
```cpp
   glm::mat4 model = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
```
这里得到的矩阵M和上面的矩阵view是相同的，可以自行验证下。
**也就是说**，通过旋转相机+y轴90度，和旋转立方体+y轴-90度，最终计算得到的矩阵相同。调整相机来得到观察效果，可以通过相应的方式来调整物体达到相同的效果。在OpenGL中并不存在真正的相机，这只是一个虚构的概念。

-----

## 视变换矩阵的推导（了解，对数学不感兴趣可跳过）

相机坐标系由相机位置eye和UVN基向量(或者说由forward, side ,up）构成，如下图所示：

![相机坐标系](http://img.blog.csdn.net/20140920173125983)
各个参数的含义如下：

- 相机位置 也称为观察参考点 (View Reference Point) 在世界坐标系下指定相机的位置eye。
- 相机镜头方向，由相机位置和相机指向的目标(target)位置计算出，$forwrad=(target - eye)$。
- 相机顶部正朝向: View Up Vector 确定在相机哪个方向是向上的，一般取(0, 1, 0)。这个参数稍后详细解释。

上面的图简化为：
![相机参数](http://img.blog.csdn.net/20160602165208232)

在使用过程中，我们是要指定的参数即为相机位置(eye)，相机指向的目标位置(target)和viewUp vector三个参数。
**Step1** : 首选计算相机镜头方向 $forwrad=(target - eye) $,
进行标准化$forward = \frac{forward}{\| forwrad\|}$。
**Step2**: 根据view-up vector和forward确定相机的side向量:
$viewUp' = \frac{viewUp}{\| viewUp\|}$
$side = cross(forward, viewUp')$

**Step3** : 根据forward和side计算up向量:
$up = cross(side, forward)$
这样eye位置，以及forward、side、up三个基向量构成一个新的坐标系，**注意**这个坐标系是一个左手坐标系，因此在实际使用中，需要对forward进行一个翻转，利用**-forward**、side、up和eye来构成一个右手坐标系。

我们的目标是计算世界坐标系中的物体在相机坐标系下的坐标，也就是从相机的角度来解释物体的坐标。从一个坐标系的坐标变换到另一个坐标系，这就是不同坐标系间坐标转换的过程。
### 计算方法1——直接计算变换矩阵
从[坐标和变换](http://blog.csdn.net/wangdingqiaoit/article/details/51394238)一节，了解到，要实现不同坐标系之间的坐标转换，需要求取一个变换矩阵。而这个矩阵就是一个坐标系A中的原点和基在另一个坐标系B下的表示。
我们将相机坐标系的原点和基，使用世界坐标系表示为(s代表side基向量，u代表up基向量，f代表forward基向量)：
$$[Camera]_{world}=\begin{bmatrix} s[0] & u[0] &  -f[0] & eye_{x} \\ 
s[1] & u[1] &  -f[1] & eye_{y} \\
s[1] & u[2] &  -f[2] & eye_{z}\\ 
0 & 0 &  0 & 1 \\
\end{bmatrix}$$
现在要求取的是坐标从世界坐标系变换到相机坐标系，则计算点p在相机坐标系下表示为：
$[p]_{camera}=[World]_{camera}[p]_{world}=[Camera]_{world}^{-1}[p]_{world}=view[p]_{world}$
即求得视变换矩阵为
$$ \begin{align} view &=[Camera]_{world}^{-1} \\
&=  \begin{bmatrix} s[0] & s[	1] & s[2] & -dot(s,eye) \\ 
u[0] & u[1] & u[2] & -dot(u, eye) \\
-f[0] & -f[1] & -f[2] & dot(f,eye) \\ 
0 & 0 & 0 & 1
\end{bmatrix} \end{align}$$
上面计算逆矩阵的过程中使用到了分块矩阵求逆矩阵的定理：
> 设方阵A、D可逆，那么分块矩阵$\begin{pmatrix} A & B \\ 0 & D\end{pmatrix}$可逆，且其逆矩阵为$T^{-1} = \begin{pmatrix} A^{-1} & -A^{-1}BD^{-1} \\ 0 & D^{-1}  \end{pmatrix}$

这种方式对应的计算代码如下：
```C++
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
```

这种方式求取过程中涉及到了分块矩阵的逆矩阵计算，如果不习惯，可以看下面的方式2，这是比较常用的方式。
### 计算方法2——利用旋转和平移矩阵求逆矩阵
求取坐标转换矩阵的过程，也可以从另外一个角度出发，即将世界坐标系旋转和平移至于相机坐标系重合，这样这个旋转$R$和平移$T$矩阵的组合矩阵$M=T*R$，就是将相机坐标系中坐标变换到世界坐标系中坐标的变换矩阵，那么所求的视变换矩阵（世界坐标系中坐标转换到相机坐标系中坐标的矩阵）$view = M^{-1}$.
其中R就是上面求得的side、up、forward基向量构成的矩阵，如下：
$$R=\begin{bmatrix} s[0] & u[0] & -f[0] & 0 \\ 
s[1] & u[1] & -f[1] & 0 \\
s[2] & u[2] & -f[2] & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}$$
$$T = \begin{bmatrix} 0 & 0 & 0 & eye_{x} \\ 
0 & 0 & 0 & eye_{y} \\
0 & 0 &0 & eye_{z} \\
0 & 0 & 0 & 1
\end{bmatrix}$$
那么所求的矩阵view计算过程如下:
$view = (T*R)^{-1} = R^{-1}*T^{-1}=R^{T}*T^{-1}$
在计算过程中，使用到了旋转矩阵的性质，即旋转矩阵是正交矩阵，它的逆矩阵等于矩阵的转置。
因此所求的:
$$R^{T}=\begin{bmatrix}
s[0] & s[1] & s[2] & 0 \\
u[0] & u[1] & u[2] & 0 \\
-f[0] & -f[1] & -f[2] & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}$$
$$T^{-1} = \begin{bmatrix} 
１ & 0 & 0 & －eye_{x} \\
0 & １ & 0 & －eye_{y} \\
0 & 0 & １& －eye_{z} \\
0 & 0 & 0&１
\end{bmatrix}$$

同样计算得到视变换矩阵为：
$$view＝R^{T}*T^{-1}　＝　\begin{bmatrix} s[0] & s[	1] & s[2] & -dot(s,eye) \\ 
u[0] & u[1] & u[2] & -dot(u, eye) \\
-f[0] & -f[1] & -f[2] & dot(f,eye) \\ 
0 & 0 & 0 & 1
\end{bmatrix}$$

这种方式对应的计算代码如下：
```C++
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
```

## OpenGL中视变换的实现
在OpenGL中，我们可以通过函数glm::lookAt来实现相机指定，这个函数计算的就是上面求出的视变换矩阵。以前glu版本实现为[gluLookAt](https://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml)，这两个函数完成的功能是一样的，参数定义如下：
> **API** lookAt ( GLdouble eyeX,  GLdouble eyeY,  GLdouble eyeZ,  GLdouble centerX,  GLdouble centerY,  GLdouble centerZ,  GLdouble upX,  GLdouble upY,  GLdouble upZ)
其中eye指定相机位置，center指定相机指向目标位置，up指定viewUp向量。

利用[GLM数学库](http://glm.g-truc.net/0.9.7/index.html)一般实现为：
```C++
glm::mat4 view = glm::lookAt(eyePos,
	glm::vec3(0.0f, 0.0f, 0.0f), 
	glm::vec3(0.0f, 1.0f, 0.0f));
```
下面利用这个函数进行一些实验，以帮助理解。在设置相机参数之前，我们学习下绘制立方体，为实验增加素材。

## 绘制立方体
前面[索引绘制矩形](http://blog.csdn.net/wangdingqiaoit/article/details/51324516)一节使用了索引了矩形，如果利用索引绘制立方体，表面上看确实可以节省顶点数据，但是存在的问题是，不能为不同面上的共同顶点指定不同的纹理坐标，这在某些情况下会出现问题的。例如下面使用索引绘制的立方体：
![索引绘制](http://img.blog.csdn.net/20160602215900766)
由于在正面和侧面的顶点制定了相同的纹理坐标，插值后纹理一致，并没有出现可爱的猫咪图案。为此，我们需要为共用顶点指定不同的顶点属性，那么解决办法之一是，继续使用顶点数组绘制方式，定义立方体的数据如下：
```C++
  // 指定顶点属性数据 顶点位置 颜色 纹理
GLfloat vertices[] = {
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A
	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,	// B
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,	    // C
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,	    // C
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// D
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A

	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// E
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0, 1.0f,    // H
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,	// G
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,	// G
	0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,	// F
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// E

	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// D
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0, 1.0f,    // H
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,	// E
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,	// E
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// D

	0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,	// F
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,	// G
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,	    // C
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,	   // C
	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,	// B
	0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,	// F

	0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,	// G
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0, 1.0f,    // H
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,	// D
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,	// D
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,	// C
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,	// G

	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	// E
	0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,	// F
	0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,	// F
	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,	// B
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// A
	};
```
着色器使用上一节绘制矩形的着色器程序。绘制立方体后，通过设定相机位置随着时间发生改变来观察这个立方体。指定相机位置为在xoz平面圆周运动的点轨迹，代码为:
```C++
GLfloat radius = 3.0f;
GLfloat xPos = radius * cos(glfwGetTime());
GLfloat zPos = radius * sin(glfwGetTime());
glm::vec3 eyePos(xPos, 0.0f, zPos);
glm::mat4 view = glm::lookAt(eyePos,
glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
```
同时在代码中指定投影方式为透视投影，代码为：
```C++
// 投影矩阵
glm::mat4 projection = glm::perspective(glm::radians(45.0f),
		(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 100.0f);
```
投影方式和投影矩阵的计算将在后面小结介绍，这里只需要知道使用方法即可。

实现绘制立方体，效果如下图所示：

![需要开启深度测试](http://img.blog.csdn.net/20160602220808184)

从上面的图中我们看到了奇怪的现象，立方体后面的部分绘制在了前面的部分上，这种现象是由于深度测试(Depth Test)未开启影响的。深度测试根据物体在场景中到观察者的距离，根据设定的glDepthFunc函数判定是否通过深度测试，默认为GL_LESS，即深度小者通过测试绘制在最终的屏幕上。关于深度测试这个主题，后面会继续学习，这里不再展开。
OpenGL中开启深度测试方法：
```C++
  glEnable(GL_DEPTH_TEST);
```
同时在主循环中，清除深度缓冲区和颜色缓冲区：
```C++
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```
开启深度测试后，旋转相机来观察立方体，效果如下：
![旋转的相机](http://img.blog.csdn.net/20160602220533540)

## viewUp向量
上面提到了在指定相机时需要指定相机的viewUP向量，这个向量指定了相机中哪个方向是向上的。对于相机而言，指定了相机位置eye和相机指向位置target后确定了相机的指向，位置不变，指向不变时，还是可以通过改变这个viewUp而影响成像的。这个类似于你眼睛的位置不变，看着的方向不变，但是你可以扭动脖子来确定哪个方向是向上，这个viewUp好比头顶给定的方向。相机位置固定在(0,0,3.0)，指向原点，依次取viewUp为$(0,1,0),(1,0,0),(0,-1,0)$，绘制立方体后的效果如下图所示：

![viewUp](http://img.blog.csdn.net/20160603172351624)

图中viewUp为(0,1,0)时猫的尾巴朝上，为(1,0,0)时相当于把脖子右旋转90度，看到猫的尾巴是在左边的；为(0,-1,0)相当于倒立过来看，猫的尾巴是向下的。如果想了解更多关于viewUp的解释，可以参考[What exactly is the UP vector in OpenGL's LookAt function](http://stackoverflow.com/questions/10635947/what-exactly-is-the-up-vector-in-opengls-lookat-function).


## 更多立方体
上一节介绍了模型变换，我们可以利用模型变换，在场景中绘制多个立方体，同时相机的位置可以采用圆的参数方程或者球面参数方程设定。绘制多个立方体的方法：
```C++
 // 指定立方体位移
 glm::vec3 cubePostitions[] = {
		glm::vec3(0.0f, 0.0f, 1.2f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.2f, 1.2f, 0.0f),
		glm::vec3(-1.2f, 1.2f, 0.0f),
		glm::vec3(-1.2f, -1.5f, 0.0f),
		glm::vec3(1.2f, -1.5f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.2f),
	};
  // 在主循环中绘制立方体
for (int i = 0; i < sizeof(cubePostitions) / sizeof(cubePostitions[0]); ++i)
{
	model = glm::mat4();
	model = glm::translate(model, cubePostitions[i]);
			glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
		1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
```
对相机位置随着时间进行改变，可以采用圆的参数方程或者球面参数方程设定。这里只是作为一个示例来设定，可以根据你的具体需求设定对应角度值。示例代码如下：
```C++
 // xoz平面内圆形坐标
glm::vec3 getEyePosCircle()
{
	GLfloat radius = 6.0f;
	GLfloat xPos = radius * cos(glfwGetTime());
	GLfloat zPos = radius * sin(glfwGetTime());
	return glm::vec3(xPos, 0.0f, zPos);
}
// 球形坐标 这里计算theta phi角度仅做示例演示
// 可以根据需要设定
glm::vec3 getEyePosSphere()
{
	GLfloat radius = 6.0f;
	GLfloat theta = glfwGetTime(), phi = glfwGetTime() / 2.0f;
	GLfloat xPos = radius * sin(theta) * cos(phi);
	GLfloat yPos = radius * sin(theta) * sin(phi);
	GLfloat zPos = radius * cos(theta);
	return glm::vec3(xPos, yPos, zPos);
}
```
例如利用球面坐标方程设定的相机位置，效果如下图所示：

![球形相机坐标](http://img.blog.csdn.net/20160602222258376)

## 最后的说明
经过视变换后，世界坐标系中坐标转换到了相机坐标系下。需要注意的相机在OpenGL中是个假想的概念，本质是通过矩阵来完成计算的。本节设定相机位置为圆周或者球面运动轨迹，并不能让用户来交互地观察场景中物体，下一节将设计一个第一人称FPS相机，让用户通过键盘和鼠标控制相机，更好地观察场景。