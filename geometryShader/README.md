写在前面
    一直以来我们使用了顶点着色器(vertex shader)和片元着色器(fragment shader)，实际上OpenGL还提供了一个可选的几何着色器(geometry shader)。几何着色器位于顶点和片元着色器之间，如果没有使用时，则顶点着色器输出到片元着色器，在使用几何着色器后，顶点着色器输出组成一个基础图元的顶点信息到几何着色器，经过几何着色器处理后，再输出到片元着色器。几何着色器能够产生0个以上的基础图元(primitive)，它能起到一定的裁剪作用、同时也能产生比顶点着色器输入更多的基础图元。本节将学习几何着色器的基本用法，示例代码均可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/geometryShader)。

> 本文整理自:
  [www.learnopengl.com Geometry Shader](http://www.learnopengl.com/#!Advanced-OpenGL/Geometry-Shader)



# 几何着色器的基本概念
几何着色器在启用后，它将获得顶点着色器以组成一个基础图元为一组的顶点输入，通过对输入的顶点进行处理，几何着色器将决定输出的图元类型和个数。当输出的图元减少或者不输出时，实际上起到了裁剪图形的作用，当输出的图元类型改变或者输出更多图元时起到了产生和改变图元的作用。
要启用几何着色器，我们需要在之前的顶点和片元着色器基础上，将几何着色器GL_GEOMETRY_SHADER链接到着色器程序上，在代码上没有太大改动，你可以从[我的github](https://github.com/wangdingqiao/noteForOpenGL/blob/master/geometryShader/geometryShader1/shader.h)查看这个头文件。在程序中，我们创建一个包含上述3中着色器的程序：

```cpp
// 准备着色器程序
Shader shader("scene.vertex", "scene.frag", "scene.gs"); 
```
# 一个直通的几何着色器
首先从一个基本的直通几何着色器来了解(以下简称gs)。这里我们绘制4个点，在gs中将这4个点的位置、大小信息原样输出到片元着色器。
**顶点着色器**如下：

```glsl
#version 330 core
layout(location = 0) in vec2 position;

void main()
{
	gl_Position = vec4(position, 0.5, 1.0);
	gl_PointSize = 2.8; // 指定点大小 需要在主程序中开启 glEnable(GL_PROGRAM_POINT_SIZE); 
}
```

**几何着色器**：

```glsl
#version 330 core
layout(points) in ;
layout(points, max_vertices = 1) out;

// 直通的几何着色器 原样输出
void main()
{
	gl_Position = gl_in[0].gl_Position;
	gl_PointSize = gl_in[0].gl_PointSize;
	EmitVertex();
	EndPrimitive();
}
```


**片元着色器**：

```glsl
#version 330 core
out vec4 color;

void main()
{
	color = vec4(0.0, 1.0, 0.0, 1.0);
}
```

-------
观察发现，在几何着色器中in和out分别指示了输入的图元，和输出的图元等参数。这里填写的是类型points表示输出点。从顶点着色器输入的图元类型，映射到几何着色器的输入模式如下表所示(参考自[OpenGL SuperBible: Comprehensive Tutorial and Reference, 6th Edition](http://www.informit.com/articles/article.aspx?p=2120983&seqNum=2))：

|几何着色器输入模式|顶点着色器输入|顶点最少个数|
|--------------|--------------|--------------|
|points |GL_POINTS|1|
|lines|GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP|2|
|triangles|GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP|3|
|lines_adjacency|GL_LINES_ADJACENCY，GL_LINE_STRIP_ADJACENCY|4|
|triangles_adjacency|GL_TRIANGLES_ADJACENCY,GL_TRIANGLE_STRIP_ADJACENCY|6|

同时从几何着色器输出模式，则有3种:

 - points
 - line_strip
 - triangle_strip

这3种模式基本包含了所有绘图类型，例如triangle_strip就包含了triangle这种特例。max_vertices表示从几何着色器最多输出顶点数目，如果超过设定的这个数目，OpenGL不会输出多余的顶点。

在上述几何着色器中EmitVertex表示输出一个顶点，而EndPrimitive表示结束一个图元的输出，这是一对命令。gl_in是内置输入变量，定义为：

```glsl
in gl_PerVertex
{
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
} gl_in[];
```
这是一个interface block，对这一概念不熟悉的可以回过头去查看[uniform block这一节的](http://blog.csdn.net/wangdingqiaoit/article/details/52717963)内容。定义输入block为一个数组，因为输入的顶点要组成一个图元，因此通常不止一个。上面的例子中，使用一个顶点，因此我们使用gl_in[0]来获取这个顶点的信息。几何着色器中内置了一个输出变量，定义如下：

```glsl
out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};
```

这是一个没有使用名字的interface block,因此在着色器中可以直接引用变量名字。

上面的输入：

```glsl
   layout(points) in ;
```
表示从顶点着色器输入GL_POINTS图元。

输出语句：

```glsl
   layout(points, max_vertices = 1) out;
```
表示从几何着色器输出points，因为是一个点，因此max_vertices选项填写1。

在主程序中，我们指定顶点数据如下：

```cpp
   // 指定顶点属性数据 顶点位置
  GLfloat points[] = {
	-0.5f, 0.5f,	// 左上
	0.5f, 0.5f,		// 右上
	0.5f, -0.5f,	// 右下
	-0.5f, -0.5f	// 左下
 };
```
使用命令：
```cpp
   glDrawArrays(GL_POINTS, 0, 4);
```
绘图后得到4个点的输出，效果如下图所示：

![原始点](http://img.blog.csdn.net/20161002215513022)


# 从点到直线

下面我们在着色器中通过将输入的一个点，产生两个发生了少许偏移的顶点，而绘制直线，着色器改为：

```glsl
   #version 330 core

layout(points) in ;
layout(line_strip, max_vertices = 2) out; // 注意输出类型
// 通过点产生直线输出
void main()
{
	gl_Position = gl_in[0].gl_Position 
	+ vec4(-0.1, 0.0, 0.0, 0.0);
	gl_PointSize = gl_in[0].gl_PointSize;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
	EmitVertex();
	EndPrimitive();
}
```
得到的效果如下图所示：

![点变直线](http://img.blog.csdn.net/20161002220826470)

# 点变为房子图案

上面产生了4条直线，我们继续产生一个triangle_strip输出，计算一个简单的房子图案的输出如下：

```glsl
   #version 330 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // 注意输出类型

void makeHouse(vec4 position)
{
    gl_Position = position + vec4(-0.2f, -0.2f, 0.0f, 0.0f);  // 左下角
	EmitVertex();
	gl_Position = position + vec4(0.2f, -0.2f, 0.0f, 0.0f);  // 右下角
	EmitVertex();
	gl_Position = position + vec4(-0.2f, 0.2f, 0.0f, 0.0f);  // 左上角
	EmitVertex();
	gl_Position = position + vec4(0.2f, 0.2f, 0.0f, 0.0f);  // 右上角
	EmitVertex();
	gl_Position = position + vec4(0.0f, 0.4f, 0.0f, 0.0f);  // 顶部
	EmitVertex();
	EndPrimitive();
}
// 输出房子样式三角形带
void main()
{
    gl_PointSize = gl_in[0].gl_PointSize;
	makeHouse(gl_in[0].gl_Position);
}
```

采用线框模式绘制得到如下图所示效果：

![点变房子图案](http://img.blog.csdn.net/20161002221055551)

在集合着色器中，我们仍然可以输出其他变量，例如颜色。我们调整下顶点属性数据，包含颜色属性，数据如下：

```cpp
   // 指定顶点属性数据 顶点位置 颜色
	GLfloat points[] = {
		-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // 左上
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, //  右上
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // 右下
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // 左下
	};
```

在顶点着色器中向几何着色器输入颜色，更改为：

```glsl
   #version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// 定义输出interface block
out VS_OUT
{
   vec3 vertColor;
}vs_out;

void main()
{
	gl_Position = vec4(position, 0.5, 1.0);
	gl_PointSize = 2.8; 
	vs_out.vertColor = color; 
}
```
在几何着色器中接受颜色输入，并调整后输出到片元着色器：

```glsl
   #version 330 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // 注意输出类型

// 定义输入interface block
in VS_OUT
{
   vec3 vertColor;
}gs_in[];

out vec3 fcolor;

void makeHouse(vec4 position)
{
   fcolor = gs_in[0].vertColor;
gl_PointSize = gl_in[0].gl_PointSize;
   gl_Position = position + vec4(-0.2f, -0.2f, 0.0f, 0.0f);  // 左下角
EmitVertex();
gl_Position = position + vec4(0.2f, -0.2f, 0.0f, 0.0f);  // 右下角
EmitVertex();
gl_Position = position + vec4(-0.2f, 0.2f, 0.0f, 0.0f);  // 左上角
EmitVertex();
gl_Position = position + vec4(0.2f, 0.2f, 0.0f, 0.0f);  // 右上角
EmitVertex();
gl_Position = position + vec4(0.0f, 0.4f, 0.0f, 0.0f);  // 顶部
fcolor = vec3(1.0f, 1.0f, 1.0f); // 这里改变顶部颜色
EmitVertex();
EndPrimitive();
}
// 输出房子样式三角形带
void main()
{
	makeHouse(gl_in[0].gl_Position);
}
```

绘制得到的房子图案如下所示：

![房子图案](http://img.blog.csdn.net/20161002221439250)

**这里我们可以发现**，从4个点的输入，通过几何着色器我们构造了4个房子图案，比原始输入产生了更多的图元，在某些场景中，这种方式能够节省CPU发往GPU的数据，从而节省带宽。


# 构造爆炸效果

几何着色器还能够产生很多有趣的效果，这里动手实践一个爆炸的效果。**实现的基本思路是**： 将模型的每个三角形，沿着这个三角形的法向量，随着时间变动，偏移一定的量offset，这个$offset >= 0.0$，则产生了爆炸效果。

在结合着色器中，首先我们需要计算法向量如下：

```glsl
   // 从输入的3个顶点 计算法向量
vec3 getNormal(vec4 pos0, vec4 pos1, vec4 pos2)
{
  vec3 a = vec3(pos0) - vec3(pos1);
  vec3 b = vec3(pos2) - vec3(pos1);
  return normalize(cross(a, b));
}
```

然后需要对输入的顶点，沿着法向量方向，偏移一定的量：

```glsl
   // 计算偏移后的三角形顶点
void explode()
{
  vec3 normal = getNormal(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
  float magnitude = ((sin(time) + 1) / 2.0f) * 2.0f; // 使位移偏量保持在[0, 2.0f]范围内
  vec4 offset = vec4(normal * magnitude, 0.0f);
  gl_Position = gl_in[0].gl_Position + offset;
  TextCoord = gs_in[0].TextCoord; // 顶点和纹理坐标每个顶点都不相同
  EmitVertex();
  gl_Position = gl_in[1].gl_Position + offset;
  TextCoord = gs_in[1].TextCoord;
  EmitVertex();
  gl_Position = gl_in[2].gl_Position + offset;
  TextCoord = gs_in[2].TextCoord;
  EmitVertex();
  EndPrimitive();
}
```

在主程序中，设置time的uniform变量：

```cpp
  glUniform1f(glGetUniformLocation(shader.programId, "time"), glfwGetTime());
```

这样随着时间变动，我们的模型的三角形顶点将发生位移，而且这个位移是向外的，因此模拟出了爆炸效果，如下图所示：

![爆炸效果](http://img.blog.csdn.net/20161002222235441)


# 绘制法向量

另外一个有用的技巧是，通过几何着色器将模型的法向量渲染出来，这样能够观察法向量是否正确，从而排查一些由于法向量指定、计算错误而导致的难以调试的错误，例如在光照计算中的法向量。

**绘制法向量基本思路是**： 绘制两遍，第一遍，用正常着色器渲染模型；第二遍，用包含了产生代表法向量方向直线的着色器再次绘制模型，这次只输出这些表示法向量的直线。在绘制代表法向量的直线时， 首先通过顶点着色器输入法向量，这个法向量需要同gl_Position一样在裁剪坐标系下。同时在几何着色器中，利用输入的法向量，为每个三角形的顶点，绘制一个直线表示这个法向量。

计算模型的法向量到裁剪坐标系，需要一些技巧，在顶点着色器中实现为：

```glsl
// 定义输出interface block
out VS_OUT
{
  vec3 normal;
}vs_out;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	// 注意这里需要向几何着色器 输出裁剪坐标系下(clip space)法向量
	// 不是世界坐标系或者相机坐标系下的法向量
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	vs_out.normal = normalize( vec3( projection * vec4(normalMatrix * normal, 1.0) ) ); // 注意再次使用normalize
}
```

注意上面代码中，最后一行的normalize需要再次调用的，否则计算出错误的法向量。 如果对于计算法向量不熟悉的话，可以回过头去查看[光照计算里面的法向量的转换](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)。

在几何着色器中，根据输入的法向量，绘制代表法向量的直线：

```glsl
   #version 330 core

layout(triangles) in ;  // 输入三角形
layout(line_strip, max_vertices = 6) out;  // 输出3个代表法向量的直线

// 定义输入interface block
in VS_OUT
{
   vec3 normal;
}gs_in[];

float magnitude = 0.1f;

// 为指定索引的顶点产生代表法向量的直线
void generateNormalLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  EmitVertex();
  vec4 offset = vec4(gs_in[index].normal * magnitude, 0.0f);
  gl_Position = gl_in[index].gl_Position + offset;
  EmitVertex();
  EndPrimitive();
}

// 输出代表法向量的直线
void main()
{
	generateNormalLine(0);
	generateNormalLine(1);
	generateNormalLine(2);
}
```
经过两次渲染，最终我们得到的效果如下图所示：

![绘制法向量](http://img.blog.csdn.net/20161002223646759)

这个效果可以用来实现模型的毛发等效果，看起来就像是身上长了毛发的效果。

值得注意的是，在顶点着色器中计算裁剪坐标系中的法向量时，最后一定要**再次使用normalize函数**，否则计算出的法向量不正确，而导致错误的效果，如下图所示：

![法向量计算出错导致的错误效果](http://img.blog.csdn.net/20161002224100698)

# 最后的说明
本节介绍了几何着色器的使用，以及基于此实现的一些特效。实际上还有其他的特效和应用，感兴趣地可以自行参考[GLSL Geometry Shaders](http://web.engr.oregonstate.edu/~mjb/cs519/Handouts/geometry_shaders.1pp.pdf)这个非常经典的文档。

# 参考资料
1.https://www.lighthouse3d.com/tutorials/glsl-tutorial/geometry-shader/
2.https://www.opengl.org/wiki/Geometry_Shader
3.http://www.informit.com/articles/article.aspx?p=2120983&seqNum=2
4.https://open.gl/geometry