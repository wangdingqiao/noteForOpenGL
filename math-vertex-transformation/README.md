写在前面
   前面几节分别介绍了[模型变换](http://blog.csdn.net/wangdingqiaoit/article/details/51531002)，[视变换](http://blog.csdn.net/wangdingqiaoit/article/details/51570001),以及给出了[投影矩阵和视口变换矩阵的推导](http://blog.csdn.net/wangdingqiaoit/article/details/51589825)，本节从全局把握一遍OpenGL坐标转换的过程，从整体上认识坐标变换过程。相关矩阵的数学推导过程请参考前面几节对应的内容。

通过本节可以了解到

- 坐标变换的各个阶段
- 利用GLM数学库实现坐标变换

## 坐标变换的全局图
OpenGL中的坐标处理过程包括模型变换、视变换、投影变换、视口变换等过程，如下图所示：
![坐标变换](http://img.blog.csdn.net/20140918202127843)

在上面的图中，注意，OpenGL只定义了裁剪坐标系、规范化设备坐标系和屏幕坐标系，而局部坐标系(模型坐标系)、世界坐标系和照相机坐标系都是为了方便用户设计而自定义的坐标系，它们的关系如下图所示(来自[Chapter 7. World in Motion](http://alfonse.bitbucket.org/oldtut/Positioning/Tutorial%2007.html))：

![坐标转换2](http://img.blog.csdn.net/20140918204835743)

图中左边的过程包括模型变换、视变换，投影变换，这些变换可以由用户根据需要自行指定，这些内容在顶点着色器中完成；而图中右边的两个步骤，包括透视除法、视口变换，这两个步骤是OpenGL自动执行的，在顶点着色器处理后的阶段完成。

## 各个变换阶段的理解
下面分别对每个阶段的变换做一个总结，以帮助理解。

----
### 模型变换——从模型坐标系到世界坐标系
局部坐标系(模型坐标系)是为了方便构造模型而设立的坐标系，建立模型时我们无需关心最终对象显示在屏幕哪个位置。模型的原点定位也可以有所不同，例如下面在模型坐标系定义的模型:
![](http://img.blog.csdn.net/20140918212527343)

模型变换的主要目的是通过变换使得用顶点属性定义或者3d建模软件构造的模型，能够按照需要，通过缩小、平移等操作放置到场景中合适的位置。通过模型变换后，物体放置在一个全局的世界坐标系中，世界坐标系是所有物体交互的一个公共坐标系。例如下面的图中在模型坐标系定义的茶壶模型(来自[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx))：

![teapot](http://img.blog.csdn.net/20140918213400484)

茶壶通过模型变换，转换到世界坐标系中(来自[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx))：

![teapot2](http://img.blog.csdn.net/20140918213635727)


模型变换包括：旋转、平移、缩放、错切等内容。例如将物体从一个位置$p=(x,y,z)$，移动到另一个位置$p'=(x',y',z')$的过程，用矩阵表示为：
$$\begin{align} p' &= Tp \\
&= \begin{bmatrix} 1 & 0 & 0 & t_{x} \\ 
0 & 1 & 0 & t_{y} \\
0 & 0 & 1 & t_{z} \\
0 & 0 & 0 & 1
\end{bmatrix} \begin{bmatrix} x \\ y \\ z \\ 1 \end{bmatrix} \\
&= \begin{bmatrix} x+t_{x} \\ y+t_{y} \\ z+t_{z} \\ 1 \end{bmatrix}
\end{align}$$
应用多个模型变换时，注意变换执行的顺序影响变换的结果，一般按照缩放--》旋转---》平移的顺序执行；另外，注意旋转和缩放变换的不动点问题。这些内容在[模型变换](http://blog.csdn.net/wangdingqiaoit/article/details/51531002)一节已经介绍了，这里不再赘述。利用[GLM数学库](http://glm.g-truc.net/0.9.7/index.html)实现模型变换，例如平移变换示例代码为：
```C++
glm::mat4 model; // 构造单位矩阵
model = glm::translate(model, glm::vec3(0.0f, 0.0f,-0.5f));
```

----
### 视变换——从世界坐标系到相机坐标系
视变换是为了方便观察场景中物体而设立的坐标系，在这个坐标系中相机是个假想的概念，是为了便于计算而引入的。相机坐标系中的坐标，就是从相机的角度来解释世界坐标系中位置。相机和场景的示意图如下所示(来自[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx))：

![camera](http://img.blog.csdn.net/20140919101312899)

OpenGL中相机始终位于原点，指向-Z轴，而以相反的方式来调整场景中物体，从而达到相同的观察效果。例如要观察-z轴方向的一个立方体的右侧面，可以有两种方式：
> 1. 立方体不动，让相机绕着+y轴，旋转+90度，此时相机镜头朝向立方体的右侧面，实现目的。完成这一旋转的矩阵记作$R_{y}(\frac{\pi}{2})$
2. 相机不动，让立方体绕着+y轴，旋转-90度，此时也能实现同样的目的。注意这时相机没有转动。完成这一旋转的矩阵记作$R_{y}(-\frac{\pi}{2})$

OpenGL中采用方式2的观点来解释视变换。再举一个例子，比如，一个物体中心位于原点，照相机也位于初始位置原点，方向指向-Z轴。为了对物体的+Z面成像，那么必须将照相机从原点移走，如果照相机仍然指向-Z轴，需要将照相机沿着+Z轴方向后退。假若照相机不移动，我们可以通过将物体沿着-Z轴后退d个单位，则变换矩阵为：
$$T = \begin{bmatrix} 1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & -d \\
0 & 0 & 0 & 1
\end{bmatrix}$$

通过**在世界坐标系中指定相机的位置**，指向的目标位置，以及viewUp向量来构造一个相机坐标系，通过视变换矩阵将物体坐标由世界坐标系转换到相机坐标系，视变换矩阵的推导已经在[视变换](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)一节介绍，感兴趣地可以去参考。利用[GLM数学库](http://glm.g-truc.net/0.9.7/index.html)实现视变换的代码为：
```C++
glm::mat4 view = glm::lookAt(eyePos,
	glm::vec3(0.0f, 0.0f, 0.0f), 
	glm::vec3(0.0f, 1.0f, 0.0f));
```

### 投影变换——从世界坐标系到裁剪坐标系
投影方式决定以何种方式成像，投影方式有很多种，OpenGL中主要使用两种方式，即透视投影([perspective projection](https://en.wikipedia.org/wiki/Perspective_(graphical)))和正交投影( [orthographic projection](https://en.wikipedia.org/wiki/Orthographic_projection_(geometry)))。
> 1.**正交投影**是平行投影的一种特殊情形，正交投影的投影线垂直于观察平面。平行投影的投影线相互平行，投影的结果与原物体的大小相等，因此广泛地应用于工程制图等方面。
2.**透视投影**的投影线相交于一点，因此投影的结果与原物体的实际大小并不一致，而是会近大远小。因此透视投影更接近于真实世界的投影方式。

两者的示意图如下：
![平行投影和透视投影](http://img.blog.csdn.net/20140921161138953)

在OpenGL中成像时的效果如下所示(图片来自[Modern OpenGL](https://glumpy.github.io/modern-gl.html))：
![投影类型](http://img.blog.csdn.net/20160605134913316)

上面的图中，红色和黄色球在视见体内，因而呈现在投影平面上，而绿色球在视见体外，没有在投影平面上成像。指定视见体通过(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)6个参数来指定。注意在相机坐标系下，相机指向-z轴，nearVal和farVal表示的剪裁平面分别为:近裁剪平面$z=−nearVal$，以及远裁剪平面$z=−farVal$。

使用
> glOrtho(xleft, xright, ybottom, ytop, znear, zfar);

或者类似API指定正交投影，参数意义形象表示为下图所示(来自[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx)):
![正交投影](http://img.blog.csdn.net/20140921162003576)

使用
> void glFrustum(GLdouble  left,  GLdouble  right,  GLdouble  bottom,  GLdouble  top,  GLdouble  nearVal,  GLdouble  farVal);
void gluPerspective(GLdouble  fovy,  GLdouble  aspect,  GLdouble  zNear,  GLdouble  zFar);

或者类似的API指定透视投影的视见体，其参数含义如下图所示(来自[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx))：

![透视投影示意图](http://img.blog.csdn.net/20140921162117984)

关于投影矩阵的推导，可以参考前面的[投影矩阵和视口变换矩阵](http://blog.csdn.net/wangdingqiaoit/article/details/51589825)一节。利用[GLM数学库](http://glm.g-truc.net/0.9.7/index.html)实现视透视投影变换的代码示例为：
```C++
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
(GLfloat)(WINDOW_WIDTH)/ WINDOW_HEIGHT, 1.0f, 100.0f);
```
经过投影变换后，物体坐标变换到了裁剪坐标系，经过OpenGL自动执行的透视除法后，变换到规范化设备坐标系中。透视除法就是将裁剪坐标系中坐标都除以$w_{c}$成分的过程。


----
### 视口变换——从NDC到屏幕坐标
视变换是将规范化设备坐标(NDC)转换为屏幕坐标的过程，如下图所示：

![视口变换](http://img.blog.csdn.net/20160605173813298)
视口变化通过函数:
[glViewport(GLint  $s_{x}$ , GLint $s_{y}$ , GLsizei $w_{s}$ , GLsizei  $h_{s}$)](https://www.opengl.org/sdk/docs/man/html/glViewport.xhtml);
[glDepthRangef(GLclampf $n_{s}$ , GLclampf $f_{s}$ )](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glDepthRangef.xml);

两个函数来指定。其中($s_{x}$,$s_{y}$)表示窗口的左下角，$n_{s}$和 $f_{s}$指定远近剪裁平面到屏幕坐标的映射关系。视口变换矩阵的推导可以参考前面的[投影矩阵和视口变换矩阵](http://blog.csdn.net/wangdingqiaoit/article/details/51589825)一节。用上面的glViewport和glDepthRangef函数指定参数后，视口变换由OpenGL自动执行，不需要额外代码。

### 坐标变换的计算过程
上述过程从坐标计算角度来看，如下图所示：
![坐标计算](http://img.blog.csdn.net/20140918210414896)

----

## 坐标变换的程序实现
在程序中，我们需要在两个部分做处理。
第一，**编写顶点着色器程序**如下：
```C++
#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 textCoord;

out vec3 VertColor;
out vec2 TextCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	VertColor = color;
	TextCoord = textCoord;
}
```
第二，**在主程序中**传递变换矩阵到顶点着色器，并绘制场景中物体，代码如下：
```C++
// 投影矩阵
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
(GLfloat)(WINDOW_WIDTH)/ WINDOW_HEIGHT, 1.0f, 100.0f);
 // 视变换矩阵
 glm::mat4 view = glm::lookAt(eyePos,
glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
 // 模型变换矩阵
glm::mat4 model = glm::mat4();
model = glm::translate(model, glm::vec3(-0.25f, 0.0f, 0.0f));
// 使用uniform变量传递MVP矩阵
glUniformMatrix4fv(
glGetUniformLocation(shader.programId, "projection"),
1, GL_FALSE, glm::value_ptr(projection)); // 传递投影矩阵
glUniformMatrix4fv(
glGetUniformLocation(shader.programId, "view"),
1, GL_FALSE, glm::value_ptr(view));// 传递视变换矩阵
glUniformMatrix4fv(
glGetUniformLocation(shader.programId, "model"),
1, GL_FALSE, glm::value_ptr(model)); // 传递模型变换矩阵
// 绘制物体
glDrawArrays(GL_TRIANGLES, 0, 36);
```
例如利用圆形坐标系指定相机位置，绘制的立方体如下图所示：

![旋转的相机](http://img.blog.csdn.net/20160602220533540)


## 参考资料
1.[World, View and Projection Transformation Matrices](http://www.codinglabs.net/article_world_view_projection_matrix.aspx)
2.[GLSL Programming/Vertex Transformations](https://en.wikibooks.org/wiki/GLSL_Programming/Vertex_Transformations)

## 相关资源
1.[OpenGL 101: Matrices - projection, view, model ](https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/)
2.[songho OpenGL Transformation](http://www.songho.ca/opengl/gl_transform.html)
3.[The Perspective and Orthographic Projection Matrix](http://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix)
4.songho [OpenGL Projection Matrix](http://www.songho.ca/opengl/gl_projectionmatrix.html)
5. [glOrtho ](https://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml)
6. [glFrustum](https://www.opengl.org/sdk/docs/man2/xhtml/glFrustum.xml)
7. [gluPerspective](https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml)
8. [gluLookAt](https://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml)