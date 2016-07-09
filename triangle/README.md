写在前面
        接着[上一节](http://blog.csdn.net/wangdingqiaoit/article/details/51308622)内容，开发环境搭建好后，我们当然想立即编写3D应用程序了。不过我们还需要些耐心，因为OpenGL是一套底层的API，因而我们要掌握的基本知识稍微多一点，在开始绘制3D图形之前，本节我们将通过绘制一个三角形的程序来熟悉现代OpenGL的概念和流程。

通过本节可以了解到:

-  缓存对象VAO和VBO
-  GLSL着色器程序的编译、链接和使用方法
-  OpenGL绘图的基本流程

## 绘图流水线简要了解
与使用高级绘图API（例如java里swing绘图，MFC里的绘图）不同，使用OpenGL绘制图形时需要对底层知识有所了解。在现代OpenGL中，完成图形绘制的流水线与旧版的固定流水线有所不同，现代OpenGL程序中允许用户自己定制着色器，这使得绘图更灵活。现代绘图流水线如下图所示(来自:[opengl wiki Rendering_Pipeline_Overview](https://www.opengl.org/wiki/Rendering_Pipeline_Overview)):
![这里写图片描述](http://img.blog.csdn.net/20160504205508618)
这个绘图流水线是比较复杂的，初学时只需要关注vertex shader顶点着色器和Fragment shader片元着色器即可。顶点着色器负责将用户指定的顶点转换为内部表示，片元着色器决定最终生成图像的颜色。顶点着色器的和片元着色器之间可以通过传递变量来沟通。使用这两个着色器就可以绘制基本的图形了，主要的流程是：
 (1) 用户在程序中指定或者加载顶点属性数据
 (2) 将顶点属性数据传送到GPU，由顶点着色器处理顶点数据
 (3) 由片元着色器负责最终图形的颜色
根据这个步骤，下面逐一熟悉相关概念和操作。

## VBO和VAO
在OpenGL程序中指定或者加载的数据是存储在CPU中的，要加快图形渲染，必定要充分利用GPU的优势，因此需要将数据发送到GPU中。在GPU中，VBO即vertex buffer object,顶点缓存对象负责实际数据的存储；而VAO即 vertex array object, 记录数据的存储和如何使用的细节信息。

OpenGL是一个状态机(state machine)，我们绘制图形时需要在不同的状态之间切换。例如上一节中通过glClearColor设置清除颜色缓冲区时设定的颜色，OpenGL则记住了这一状态，当调用glClear时则使用这个颜色重置颜色缓冲区。直到再次使用glClearColor设置不同颜色为止，OpenGL会一直使用这个状态值。

使用VAO的优势就在于，如果有多个物体需要绘制，那么我们设置一次绘制物体需要的顶点数据、数据解析方式等信息，然后通过VAO保存起来后，后续的绘制操作不再需要重复这一过程，只需要将VAO设定为当前VAO，那么OpenGL则会使用这些状态信息。当场景中物体较多时，优势十分明显。VAO和VBO的关系如下图所示(图片来自[Best Practices for Working with Vertex Data](https://developer.apple.com/library/ios/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/TechniquesforWorkingwithVertexData/TechniquesforWorkingwithVertexData.html)):
![这里写图片描述](http://img.blog.csdn.net/20160505104032500)
上图中表示，顶点属性包括位置、纹理坐标、法向量、颜色等多个属性，每个属性的数据可以存放在不同的buffer中。我们可以根据需求，在程序中创建多个VBO和VAO。

使用VAO和VBO的伪代码如下(来自[SO](http://stackoverflow.com/questions/8923174/opengl-vao-best-practices)):
```C++
initialization:
    for each batch
        generate, store, and bind a VAO
        bind all the buffers needed for a draw call
        unbind the VAO

main loop/whenever you render:
    for each batch
        bind VAO
        glDrawArrays(...); or glDrawElements(...); etc.
    unbind VAO
```

那么如何创建VBO和VAO呢？ OpenGL中的对象创建和使用与C++中对象创建不一样，下面代码描述了在C++中创建和使用对象的方式(来自[Learning Modern 3D Graphics Programming]):
```C++
struct Object  
{  
    int count;  
    float opacity;  
    char *name;  
};  
  
//创建对象.  
Object newObject;  
  
// 设置对象的状态
newObject.count = 5;  
newObject.opacity = 0.4f;  
newObject.name = "Some String";
```
在OpenGL中创建和使用对象却类似这样:
```C++
//创建对象 不允许使用自定义名字  
GLuint objectName;  
glGenObject(1, &objectName);  
  
// 设置对象状态
glBindObject(GL_MODIFY, objectName);  
glObjectParameteri(GL_MODIFY, GL_OBJECT_COUNT, 5);  
glObjectParameterf(GL_MODIFY, GL_OBJECT_OPACITY, 0.4f);  
glObjectParameters(GL_MODIFY, GL_OBJECT_NAME, "Some String"); 
```
注意OpenGL中创建一个对象，由GLuint 类型来作为对象标识符，而不允许使用自定义名字，这样就不会导致对象重名了。在OpenGL中每个对象在使用前，要绑定到上下文对象,即所谓的target，例如上例中就是GL_MODIFY这个target。
**Step1**: 创建VBO  我们这样来创建:
```C++
    GLuint VBOId;
    glGenBuffers(1, &VBOId);
```
> **API** [ void glGenBuffers(	GLsizei  	n, GLuint *  	buffers)](http://docs.gl/gl3/glGenBuffers);
> 这里n指定产生buffer的数目，而buffers则是标识符的地址。一次可以产生一个或者多个buffer.

**Step2**: 将顶点数据传送到VBO或者为VBO预分配空间。
本节我们绘制一个三角形，对于三角形要在3D空间中指定顶点，必定使用三维坐标。这个顶点坐标需要经过顶点着色器处理后最终才能用于生产三角形，这里面涉及到坐标转换等内容，本节不做深入探讨。经过坐标转换后，顶点坐标最终落在规范化设备坐标系(normalized device coordinate , NDC)中, NDC中坐标范围均为[-1,1]，因此这里我们简化处理，将顶点坐标全部定在这个范围内，指定为:
```C++
GLfloat vertices[] = {
	-0.5f, 0.0f, 0.0f,
	0.5f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f
};
```
将数据传送到GPU中需要通过函数[glBufferData](https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml)实现。
>**API** void glBufferData( 	GLenum target,
  	  GLsizeiptr size,
  	  const GLvoid * data,
  	  GLenum usage);
> 1.函数中target参数表示绑定的目标，包括像GL_ARRAY_BUFFER用于Vertex attributes(顶点属性)，GL_ELEMENT_ARRAY_BUFFER用于索引绘制等目标。
> 2.size参数表示需要分配的空间大小，以字节为单位。
> 3.data参数用于指定数据源，如果data不为空将会拷贝其数据来初始化这个缓冲区，否则只是分配预定大小的空间。预分配空间后，后续可以通过[glBufferSubData](https://www.opengl.org/sdk/docs/man2/xhtml/glBufferSubData.xml)来更新缓冲区内容。
> 4.usage参数指定数据使用模式，例如GL_STATIC_DRAW指定为静态绘制，数据保持不变, GL_DYNAMIC_DRAW指定为动态绘制，数据会经常更新。

我们这里绘制一个静态的三角形，vertex attribute顶点属性这个概念包括顶点的位置、纹理坐标、法向量、颜色等属性数据，因此我们的顶点位置数据适合绑定到GL_ARRAY_BUFFER目标，同时数据在传送时初始化缓冲区，因此可以这样实现:
``` C++
   glBindBuffer(GL_ARRAY_BUFFER, VBOId);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```
**Step3:** 通知OpenGL如何解释这个顶点属性数组
将数据传送到GPU后，我们还需要告知OpenGL如何解释这个数据，也就是告知其数据格式，因为从底层来看数据一个字节块而已。要通知OpenGL如何解释数据，要使用函数[glVertexAttribPointer](https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml).

> **API**  void glVertexAttribPointer( 	GLuint index,
	  	GLint size,
	  	GLenum type,
	  	GLboolean normalized,
	  	GLsizei stride,
	  	const GLvoid * pointer);
     1. 参数index 表示顶点属性的索引 这个索引即是在顶点着色器中的属性索引，索引从0开始记起。
   2. 参数size 每个属性数据由几个分量组成。例如上面顶点每个属性为3个float组成的，size即为3。分量的个数必须为1,2,3,4这四个值之一。
   3. 参数type表示属性分量的数据类型，例如上面的顶点数据为float则填写GL_FLOAT.
   4. 参数normalized 表示是否规格化，当存储整型时，如果设置为GL_TRUE,那么当被以浮点数形式访问时，有符号整型转换到[-1,1],无符号转换到[0,1]。否则直接转换为float型，而不进行规格化。
   5. 参数stride表示连续的两个顶点属性之间的间隔，以字节大小计算。
   6. 参数pointer表示当前绑定到 GL_ARRAY_BUFFER缓冲对象的缓冲区中，顶点属性的第一个分量距离数据的起点的偏移量，以字节为单位计算。

上面这个函数是很重要的，刚接触时可能对多个参数感到厌烦，慢慢就会习惯。这里以上述包含顶点位置的属性数组为例，做一个图解(来自:[learn opengl](http://www.learnopengl.com/#!Getting-started/Hello-Triangle)):
![这里写图片描述](http://img.blog.csdn.net/20160504222103420)
这里我们可以看出，调用上述函数时，属性索引为0(稍后着色器中会与之对应), 属性的分量个数为3，分量的数据类型为GL_FLOAT, normalized设为GL_FALSE, 参数stride为3*sizeof(GL_FLOAT)=12,
pointer的偏移量为0，但是要写为(GLvoid*)0(强制转换)，具体如下所示:
```C++
glVertexAttribPointer(0, 3, GL_FLOAT, 
	 3 * sizeof(GL_FLOAT), (GLvoid*)0);
glEnableVertexAttribArray(0);
```
这样我们创建了VBO，并将数据传送到GPU，并告知了OpenGL如何解析这些数据。在整个过程中，我们调用了很多函数，如果在以后绘制时好需要继续调用这些函数，那将会多么麻烦，因此这时候VAO就起到了关键作用。VAO能记录VBO的相关信息，在以后绘图时，只需要绑定对应的VAO就能找到这些状态，方便OPenGL使用。因此，在创建VBO这一过程中，我们要使用VAO来记录。方法便是，在所有VBO操作之前，先创建和绑定VAO。

绘制三角形时创建VAO和VBO的最终的代码如下:
```C++
    // 指定顶点属性数据 顶点位置
	GLfloat vertices[] = {
		-0.5f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f
	};
	// 创建缓存对象
	GLuint VAOId, VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO对象
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	// Step3: 分配空间 传送数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Step4: 指定解析方式  并启用顶点属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 解除绑定
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
```
 在代码的最后，我们暂时解除绑定，能够防止后续操作干扰到了当前VAO和VBO。
 现在在程序中使用VAO绘制三角形则只需要调用:
 
```C++
	glBindVertexArray(VAOId); // 使用VAO信息
	glUseProgram(shaderProgramId); // 使用着色器
	glDrawArrays(GL_TRIANGLES, 0, 3);
```

 这里使用着色器，稍后介绍。[glDrawArrays](https://www.opengl.org/sdk/docs/man/html/glDrawArrays.xhtml)函数使用VBO数据绘制物体。其使用方法为:
 > **API** void glDrawArrays( 	GLenum mode,
  	GLint first,
  	GLsizei count);
  	1.mode 参数表示绘制的基本类型，OpenGL预制了 GL_POINTS, GL_LINE_STRIP等基本类型。一个复杂的图形，都是有这些基本类型构成的。
  	2.first表示启用的顶点属性数组中第一个数据的索引。
  	3.count表示绘制需要的顶点数目。
  	
 上述调用时我们选择GL_TRIANGLES表示绘制三角形，使用3个顶点。

## 着色器程序
目前我们主要使用顶点着色器和片元着色器。对于着色器，采用的是GLSL语言([OpenGL Shading Language](https://www.opengl.org/wiki/OpenGL_Shading_Language))编写的程序，类似于C语言程序。
要使用着色器需要经历3个步骤:

1. 创建和编译shader object
2. 创建shader program,链接多个shader object到program
3. 在绘制场景时启用shader program

具体流程如下图所示:
![这里写图片描述](http://img.blog.csdn.net/20140829165138584)

我们这里写一个简单的直通着色器，在顶点着色器中输出传入的顶点位置，在片元着色器中输出指定颜色。实际应用中这两个程序将决定图形最终效果，这里只是做一个简单示例。

顶点着色器代码为:
```C++
#version 330   // 指定GLSL版本3.3

layout(location = 0) in vec3 position; // 顶点属性索引

void main()
{
	gl_Position = vec4(position, 1.0); // 输出顶点
}
```
其中gl_Position为内置变量，表示顶点输出位置，以gl_前缀开头的一般都表示内置变量。position声明为vec3类型， vec3表示3个float类型的向量。gl_Positon为vec4类型，其中第四个分量为1.0,关于这个分量后面会做介绍。

片元着色器代码为:
```C++
#version 330

out vec4 color; // 输出片元颜色

void main()
{
	color = vec4(0.8, 0.8, 0.0, 1.0);
}
```
通过color指定最终颜色为黄色，vec4类型表示颜色为RGB再加上alpha值构成最终的输出颜色。关于alpha值后面会介绍。

首先创建顶点和片元着色器对象，要注意其中的错误处理。其中顶点着色器代码如下：
```C++
   const GLchar* vertexShaderSource = "#version 330\n"
		"layout(location = 0) in vec3 position;\n"
		"void main()\n"
		"{\n gl_Position = vec4(position, 1.0);\n}";
	// Step2 创建Shader object
	// 顶点着色器
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderId);
	GLint compileStatus = 0;
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &compileStatus); // 检查编译状态
	if (compileStatus == GL_FALSE) // 获取错误报告
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errLog(maxLength);
		glGetShaderInfoLog(vertexShaderId, maxLength, &maxLength, &errLog[0]);
		std::cout << "Error::shader vertex shader compile failed," << &errLog[0] << std::endl;
	}
```
片元着色器也有类似处理，然后创建并连接，形成shader program对象，代码如下:
``` C++
    GLuint shaderProgramId = glCreateProgram();// 创建program
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragShaderId);
	glLinkProgram(shaderProgramId);
	GLint linkStatus;
	glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(shaderProgramId, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errLog(maxLength);
		glGetProgramInfoLog(shaderProgramId, maxLength, &maxLength, &errLog[0]);
		std::cout << "Error::shader link failed," << &errLog[0] << std::endl;
	}
```

**注意**: 在shader object链接到program后，即可断开链接，如果不需要再链接到其他program，比较好的做法就是释放资源：
```C++
        // 链接完成后detach
	glDetachShader(shaderProgramId, vertexShaderId);
	glDetachShader(shaderProgramId, fragShaderId);
	// 不需要连接到其他程序时 释放空间
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragShaderId);
```

## 绘制三角形
通过上面使用VAO和VBO完成了数据存储和解析部分工作，通过着色器完成了图形的渲染，将这两个部分组成一起，我们便可以绘制我们的三角形了。运行结果如下图所示:

![这里写图片描述](http://img.blog.csdn.net/20160504225801293)
程序的完整代码可以[从github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/triangle/triangle)。

注意如果着色器程序失败，我们得到的图形如下图所示:

![这里写图片描述](http://img.blog.csdn.net/20160504225915653)

失败时请检查着色器代码部分。

## 重构代码
将上述着色器代码，分装成一个shader类，这个类从文件读取着色器源码，并创建着色器程序，是代码更简洁。使用shader类的项目结构为:
![这里写图片描述](http://img.blog.csdn.net/20160504230234029)
使用着色器类创建着色器代码简化为:
```c++
Shader shader("triangle.vertex", "triangle.frag");
```
着色器程序只读取程序源码，与文件名称和类型无关。
使用封装的类实现的三角形绘制版本和shader类的代码可以[从github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/triangle/shaderClass)。
## 添加顶点的颜色属性
上面绘制的三角形，使用的颜色是在片元着色器中指定的，我们可以通过vertex attribute指定顶点颜色属性，同顶点位置属性一样传送给着色器。修改顶点属性数组数据为：
``` C++
GLfloat vertices[] = {
		// 顶点坐标   顶点颜色
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};
```
我们需要重新指定OpenGL解析数据的方式，需要更新着色器。
**Step1**: 首先通过glVertexAttribPointer重新解释数据，代码变为:
```c++
// Step4: 指定解析方式  并启用顶点属性
// 顶点位置属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
	6 * sizeof(GL_FLOAT), (GLvoid*)0);
glEnableVertexAttribArray(0);
// 顶点颜色属性
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
	6 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
glEnableVertexAttribArray(1);
```
这里注意glVertexAttribPointer的参数设定，stride和pointer参数解释如下图所示(来自[www.learnopengl.com](http://www.learnopengl.com/#!Getting-started/Shaders)):
![这里写图片描述](http://img.blog.csdn.net/20160505105533093)
即顶点位置和颜色的stride即都为 6 * sizeof(GL_FLOAT) = 24, 顶点位置数据首地址偏移量为0，而颜色数据首地址偏移量为 3 * sizeof(GL_FLOAT) = 12。

**Step2 ** 更新着色器，在顶点着色器中为颜色属性指定索引为1，更新后的顶点着色器为:
```c++
#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 vertColor;

void main()
{
	gl_Position = vec4(position, 1.0);
	vertColor = color; // 输出顶点颜色
}
```
这里通过定义location = 1将颜色属性索引设为1，同时直接输出到片元着色器。片元着色器为:
```C++
   #version 330

in vec3 vertColor;
out vec4 color;

void main()
{
	color = vec4(vertColor, 1.0);
}
```
片元着色器接受顶点着色器输出的颜色vertexColor，然后直接作为最终片元颜色。注意在顶点着色器和片元着色器之间传递变量时，要求变量的类型和名字必须一致，例如这里的vertexColor变量。
更新后的程序，运行效果如下图所示:
![这里写图片描述](http://img.blog.csdn.net/20160505110100146)

完整的代码可以[从github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/triangle/coloredTriangle)。
如上图所示，我们在顶点属性中指定了红绿蓝三个颜色，实际在产生图形时生成更多的片元，这些片元的颜色是通过颜色插值（Color Interpolation）来生成的。颜色插值的一种方法是线性插值，例如一条直线一端点指定为红色，另一端点指定为绿色，则位于中间部分点的颜色，可以按照公式:
$ y = a +(b-a)*t$
来生成颜色。当t=0时，取值为a表示颜色为红色，t=1时取值为b表示为绿色，当t取值在0和1之间时，则按照比例混合红色和绿色生成最终的颜色。上图中除了顶点以外的颜色，其余部分都是通过颜色插值得来的。颜色插值理论后面还要深入了解。

## 使用模板快速获取本节工程
有网友留言索要整个工程，因为github上面上传二进制的VS工程不太合适，这里制作了一个方便的模板供使用，可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/getting-started-template)。模板使用方法：

**Step1**: 将模板getting-started.zip拷贝值VS的项目模板目录，如下图所示：
![安装模板](http://img.blog.csdn.net/20160709103324246)


**Step2**: 使用模板新建工程，如下图：

![新建工程](http://img.blog.csdn.net/20160709103402880)

**Step3**: 将libraries拷贝至新建项目的同级目录下。
![拷贝libraries](http://img.blog.csdn.net/20160709103245051)

**Step4**: 编译运行新建工程即可。

## 了解更多
1. 着色器编译和链接 [Shader Compilation](https://www.opengl.org/wiki/Shader_Compilation)
2. VAO和VBO 等缓存对象 [Buffer Objects Advanced](http://www.informit.com/articles/article.aspx?p=1377833&seqNum=7)
3. 绘图流水线 [The Graphics Pipeline](http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-1:-The-Graphics-Pipeline.html)
以及 [CG_BasicsTheory](https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_BasicsTheory.html)

##参考资料
1.Hello Triangle [learnOpenGL.com](http://www.learnopengl.com/#!Getting-started/Hello-Triangle)