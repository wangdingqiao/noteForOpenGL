写在前面
   OpenGL中还包含除了我们前面介绍的VAO,VBO,EBO等其他类型的缓冲对象。关于如何使用这些缓冲对象的手册或者参考书籍上解释得非常详细，但是**阅读起来确实很枯燥无味**。
   本文将通过简洁、可靠的例子说明一些重要方法的使用，以辅助学习这些方法。**本文的目的不是写成详细而厚重的手册**，对于文中未详细说明的部分，请参考官方文档。
   学习这些内容的过程需要随着实践慢慢积累，因此本文也将随着实践进行，不断更新。本节示例代码均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/bufferObject)。


# buffer object的概念

在OpenGL中，缓存对象(buffer object)是由OpenGL的绘制上下文(context)，也即是GPU分配的，存储未格式化的数据的区域，例如存储顶点数据，帧缓冲数据等。

缓存对象中的数据实际上就是原始的字节流，例如我们在程序中指定顶点属性数据，然后使用glBufferData​这个函数将数据从CPU中传送至GPU中，这一步只完成了数据的上传，关于这份数据，OpenGL是不知道其具体格式的，因此在实际使用中还必须告诉OpenGL如何具体使用这些数据，这是通过glVertexAttribPointer这样的函数来完成的。

每个缓存对象，必须绑定到一个具体的目标(target)后，OpenGL才知道如何具体操作这个缓存对象，例如绑定到GL_ARRAY_BUFFER，OpenGL使用这个缓存对象作为顶点属性数据；例如绑定到GL_ELEMENT_ARRAY_BUFFER，则OpenGL使用这个缓存对象的数据作为索引绘图的索引数据。

缓存对象的数据，也存在读写、复制、清除等操作，同时缓存对象在适当时候也可以使用 glDeleteBuffers​这类函数释放。

下面通过简洁的例子介绍一些重要方法的使用。

# glVertexAttribPointer

这个函数的使用容易引起一些混淆，尤其是末尾两个参数。

> **API** void glVertexAttribPointer( 	GLuint index,
  	GLint size,
  	GLenum type,
  	GLboolean normalized,
  	GLsizei stride,
  	const GLvoid * pointer);
    1. 参数index 表示顶点属性的索引 这个索引即是在顶点着色器中的属性索引，索引从0开始记起。
   2. 参数size 每个属性数据由几个分量组成。例如上面顶点每个属性为3个float组成的，size即为3。分量的个数必须为1,2,3,4这四个值之一。
   3. 参数type表示属性分量的数据类型，例如上面的顶点数据为float则填写GL_FLOAT.
   4. 参数normalized 表示是否规格化，当存储整型时，如果设置为GL_TRUE,那么当被以浮点数形式访问时，有符号整型转换到[-1,1],无符号转换到[0,1]。否则直接转换为float型，而不进行规格化。
   5. 参数stride表示连续的两个顶点属性之间的间隔，以字节大小计算。当顶点属性紧密排列时，可以用初始值为0，这时OpenGL将自动推算出stride。
   6. 参数pointer表示当前绑定到的 GL_ARRAY_BUFFER缓冲区中，顶点属性的第一个分量在数据中的偏移量，以字节为单位计算。初始值为0。

这个函数末尾的两个参数，stride表示缓存对象中连续两个顶点属性之间的间隔，而offset则表示顶点属性的第一个分量在这儿缓存对象数据中的位置偏移量。具体的计算第$i$个分量的方式为(参考自[SO](http://stackoverflow.com/questions/36229863/glvertexattribpointer-vec2-but-forward-just-one/36230568))：

$ startPos(i) = offset + i*stride \tag{顶点属性位置计算}$

例如下图中给定的交错形式的顶点属性数据，对应的顶点属性位置如下：

![顶点属性位置计算](http://img.blog.csdn.net/20160925160200175)

例如计算第三个颜色rgb的开始位置为：
$ 12+ 2* (32) = 76 $。

# 三种传送顶点属性数据的方式

这里以绘制一个三角形为例，这个三角形包含的顶点属性包括：位置、颜色、以及纹理坐标，着色器中将颜色和纹理进行混合。

有了上面对glVertexAttribPointer的理解，我们可以有三种方式传送顶点数据：

- 为每个顶点属性指定一个独立的VBO

- 单个顶点属性连续存放，整个顶点属性作为一个一个VBO

- 顶点属性之间交错存放，整体作为一个VBO

这三种方式都是支持的，通过这三种方式的实现，我们加深对glVertexAttribPointer的理解。

## 方式一 指定多个VBO的方式

存放格式为： (xyzxyz)(rgbrgb) (stst)
实现代码为:

```cpp
   // 顶点属性每个独立的VBO
void prepareVertData_moreVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性独立
GLfloat vertPos[] = {
		-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
	};
GLfloat vertColor[] = {
	1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
};
GLfloat vertTextCoord[] = {
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
};
	// 创建缓存对象
	GLuint VBOId[3];
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	glGenBuffers(3, VBOId);
	// 顶点位置属性
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos), vertPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertColor), vertColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glBindBuffer(GL_ARRAY_BUFFER, VBOId[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertTextCoord), vertTextCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId[0]);
	VBOIdVec.push_back(VBOId[1]);
	VBOIdVec.push_back(VBOId[2]);
}
```
**注意**这种方式下stride和offset参数都填写为0，由OpenGL推算出stride值，例如颜色属性的stride推算为: 3 * sizeof(GLfloat) = 12。

## 方式二  连续存放作为一个VBO

格式为： (xyzxyz... rgbrgb... stst...)
实现为:

```cpp
   // 顶点属性单个连续的整体VBO
void prepareVertData_seqBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性连续作为整体
	GLfloat vertices[] = {
-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 传送数据
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Step3: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 紧密排列 stride也可以设置为 3 * sizeof(GL_FLOAT)
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(18 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	VBOIdVec.push_back(VBOId);
}
```

**注意** 这里单个属性依然是紧密排列，所以stride值可以填写0；但是属性之间的offset需要手动计算出来，不能再填写为0了。

## 方式三  交错排列使用一个VBO
格式为：（xyzrgbst xyzrgbst...）
实现为：

```cpp
   // 顶点属性之间交错的整体VBO
void prepareVertData_interleavedBatchVBO(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
	// 顶点位置 颜色 纹理纹理
	-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
	0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 传送数据
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Step3: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT),(GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}
```
**需要注意的是**这里的stride和offset都需要手动计算出来，不能依赖默认值0了。如果stride这里填写0，OpenGL将无法推算出来，导致渲染不出来三角形。

对于这三种方式的性能对比，尚没有做测试。OpenGL官方文档描述性能差异不明确。根据需要，你可以选取其中的一种方式来实现你要的功能。

上面绘制的混色三角形，如下图所示：

![混色三角形](http://img.blog.csdn.net/20160925165815735)

# 使用glBufferSubData

前面都是分配了一个VBO，然后将顶点属性数据直接拷贝到整个VBO，实际上可以先为VBO分配空间，然后根据需要更新VBO中一段内容，这里使用到的方法就是[glBufferSubData](https://www.opengl.org/sdk/docs/man/html/glBufferSubData.xhtml)。

> **API**  void glBufferSubData( 	GLenum target,
  	GLintptr offset,
  	GLsizeiptr size,
  	const GLvoid * data);
  	1.target 绑定到的目标对象
  	2.在缓存对象中的偏移
  	3.指定更改的数据大小
  	4.用指针指定原始数据的位置

例如上面绘制三角形例子中，我们先分配一个大的VBO，然后将多个顶点属性分配拷贝至这个VBO，实现为:

```cpp
   // 使用glBufferSubData完成VBO
void prepareVertData_glBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 单个属性独立
	GLfloat vertPos[] = {
		-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // 顶点位置
	};
	GLfloat vertColor[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // 颜色
	};
	GLfloat vertTextCoord[] = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f     // 纹理坐标
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象 预分配空间
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos)+sizeof(vertColor) + sizeof(vertTextCoord), 
		NULL, GL_STATIC_DRAW);  // 预分配空间
	// Step3: 填充具体的数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertPos), vertPos);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos), sizeof(vertColor), vertColor);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos)+ sizeof(vertColor), sizeof(vertTextCoord), vertTextCoord);
	// Step4: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 紧密排列 stride也可以设置为 3 * sizeof(GL_FLOAT)
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(18 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}
```

注意代码中使用glBufferData的第三个参数为NULL时，为预分配空间，在使用glBufferSubData时一定要预分配空间。

# glMapBuffer

有些时候我们想直接获取缓存对象的数据空间，这时可以通过glMapBuffer这个函数和其配对的[glUnmapBuffer](https://www.opengl.org/sdk/docs/man/html/glMapBuffer.xhtml)函数实现。

> **API** void *glMapBuffer( 	GLenum target,
  	GLenum access);
  	1.指定要映射的目标对象
  	2.指定映射的读写策略，为枚举值GL_READ_ONLY, GL_WRITE_ONLY，或者 GL_READ_WRITE。

例如上面绘制三角形中，我们将GL_ARRAY_BUFFER映射后，获取到缓存对象的内存指针后，我们将数据拷贝到GL_ARRAY_BUFFER中，实现为：

```cpp
   // 使用glMapBuffer
void prepareVertData_glMapBuffer(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
		// 顶点位置 颜色 纹理纹理
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	// Step3: 映射数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 预分配空间
	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!ptr)
	{
		std::cerr << " map buffer failed!" << std::endl;
		return;
	}
	memcpy(ptr, vertices, sizeof(vertices));  // 可以操作这个指针指向的内存空间
	glUnmapBuffer(GL_ARRAY_BUFFER);
	// Step4: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}
```

需要注意的是如果要拷贝数据到这个映射后的内存指针指向的空间上，一定要提前为缓存对象预分配空间，否则将无法复制数据。


# glCopyBufferSubData

[glCopyBufferSubData](https://www.opengl.org/sdk/docs/man/html/glCopyBufferSubData.xhtml)这个函数可以用于缓存对象之间拷贝数据。

> **API** void glCopyBufferSubData( 	GLenum readTarget,
  	GLenum writeTarget,
  	GLintptr readOffset,
  	GLintptr writeOffset,
  	GLsizeiptr size);
    1.readTarget指定读数据的目标
    2.writeTraget指定写数据的目标
    3.readOffset指定读取数据时的起始偏移量
    4.writeOffset指定写入数据时的偏移量
    5.size指定拷贝数据的大小

作为函数方法使用的一个演示，我们将顶点属性数据首先通过glMapBuffer读入到GL_COPY_WRITE_BUFFER，然后通过glCopyBufferSubData将数据从GL_COPY_WRITE_BUFFER拷贝到GL_ARRAY_BUFFER。实现为：

```cpp
   // 使用glCopyBufferSubData 
void prepareVertData_glCopyBufferSubData(GLuint& VAOId, std::vector<GLuint>& VBOIdVec)
{
	// 交错指定顶点属性数据
	GLfloat vertices[] = {
	// 顶点位置 颜色 纹理纹理
	-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0
	0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 1
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // 2
	};
	// 创建缓存对象
	GLuint VBOId;
	// Step1: 创建并绑定VAO对象
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	// Step2: 创建并绑定VBO 对象
	glGenBuffers(1, &VBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	GLuint TempBufferId;
	glGenBuffers(1, &TempBufferId);
	glBindBuffer(GL_COPY_WRITE_BUFFER, TempBufferId);
	// Step3: 映射数据到GL_COPY_WRITE_BUFFER
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 预分配空间
	void* ptr = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	if (!ptr)
	{
		std::cerr << " map buffer failed!" << std::endl;
		return;
	}
	memcpy(ptr, vertices, sizeof(vertices));  // 可以操作这个指针指向的内存空间
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	// Step4: 将数据拷贝到VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW); // 注意拷贝前预分配空间
	glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_ARRAY_BUFFER, 0, 0, sizeof(vertices));
	glDeleteBuffers(1, &TempBufferId);
	// Step5: 指定解析方式  并启用顶点属性
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	VBOIdVec.push_back(VBOId);
}
```

后面还有其他方法，等待实践， 未完待续...


# 参考资料
1.[OpenGL SuperBible: Comprehensive Tutorial and Reference](https://books.google.com.hk/books?id=odgdAAAAQBAJ&pg=PA101&lpg=PA101&dq=glvertexattribpointer+tightly+packed&source=bl&ots=waRtFWfTI7&sig=jrPIXHBKLasYZhPHlvhCvkpcfvE&hl=en&sa=X&ved=0ahUKEwi-usju1KTPAhXEJh4KHdwZBFU4ChDoAQgjMAI#v=onepage&q=glvertexattribpointer%20tightly%20packed&f=false)
2.[OpenGL Programming Guide: The Official Guide to Learning OpenGL, Version 4.3](https://books.google.com.hk/books?id=jG4LGmH5RuIC&pg=PT207&lpg=PT207&dq=glvertexattribpointer+tightly+packed&source=bl&ots=q2HMw5lEv1&sig=986LfZPGz2yqbiImg-vtopIpH2w&hl=en&sa=X&ved=0ahUKEwiXpPHi06TPAhUFlh4KHbN4ApQQ6AEIVTAJ#v=onepage&q=glvertexattribpointer%20tightly%20packed&f=false)
3.[Vertex Specification](https://www.opengl.org/wiki/Vertex_Specification)
4.[Data in OpenGL Buffers](http://www.informit.com/articles/article.aspx?p=2033340&seqNum=2)
5.[Buffer Object](https://www.opengl.org/wiki/Buffer_Object)
6.[Vertex Specification Best Practices](https://www.opengl.org/wiki/Vertex_Specification_Best_Practices)