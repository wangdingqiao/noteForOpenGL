写在前面
         目前，我们在着色器中要传递多个uniform变量时，总是使用多个uniform，然后在主程序中设置这些变量的值；同时如果要在多个shader之间共享变量，例如投影矩阵projection和视变换矩阵view的话，仍然需要为不同shader分别设置这些uniform变量。本节将为大家介绍interface block，以及基于此的uniform buffer object(UBO)，这些技术将简化着色器中变量的传递和共享问题。本节示例程序均可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/uniformBufferObject)。

> 本节内容参考自:
1.[www.learningopengl.com Advanced GLSL](http://www.learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL)
2.[GLSL Tutorial – Uniform Blocks](http://www.lighthouse3d.com/tutorials/glsl-tutorial/uniform-blocks/)
3.[《OpenGL 4.0 Shading Language Cookbook》-Using Uniform Blocks and Uniform Buffer Objects](https://www.packtpub.com/books/content/opengl-40-using-uniform-blocks-and-uniform-buffer-objects)


# interface block
[interfac block](https://www.opengl.org/wiki/Interface_Block_(GLSL))是一组GLSL着色器里面的输入、输出、uniform等变量的集合，有一些类似于C语言中的struct，但是不像struct那样简单明了，还有一些其他的选项包含在里面。通过使用interface block，我们可以将着色器中的变量以组的形式来管理，这样书写更整洁。

interface block的声明形式为：

```
storage_qualifier block_name
{
  <define members here>
} instance_name;
```
其中storage_qualifier指明这个block的存储限定符，限定符可以使用in​, out​, uniform​, 或者buffer​（GLSL4.3支持）等，block_name则给定名称，而instance_name给定实例名称。

例如，我们[之前在实现点光源](http://blog.csdn.net/wangdingqiaoit/article/details/51867538)的过程中，顶点着色器和片元着色器之间需要传递法向量、纹理坐标等变量，将他们封装到一个block中，代码显得更紧凑。顶点着色器中输出变量定义形式如下：

```glsl
// 定义输出interface block
out VS_OUT
{
	vec3 FragPos;
	vec2 TextCoord;
	vec3 FragNormal;
}vs_out;
```
而在片元着色器中，要以相同的block_name接受，实例名称则可以不同，形式可以定义为：

```glsl
// 定义输入interface block
in VS_OUT
{
	vec3 FragPos;
	vec2 TextCoord;
	vec3 FragNormal;
}fs_in;
```
如果指定了instance_name，则在片元着色器中引用这些变量时需要加上instance_name前缀，例如：

```glsl
   // 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, fs_in.TextCoord));
```
反之如果没有指定instance_name,则这个block中的变量将和uniform一样是全局的，可以直接使用。如果没有给定instance_name,则需要注意，interface block中给定的变量名不要和uniform给定的重复，否则造成重定义错误，例如下面的定义将造成重定义错误：

```glsl
uniform MatrixBlock
{
  mat4 projection;
  mat4 modelview;
};

uniform vec3 modelview;  // 重定义错误 和MatrixBlock中冲突

```

相比于之前以分散形式书写这些变量，interface block能够让你更合理的组织变量为一组，逻辑更清晰。

主程序部分未变，实现的点光源效果相同，这里还是给出效果图如下：

![这里写图片描述](http://img.blog.csdn.net/20161001163155279)

从上面可以看出，interface block确实解决了我们一直想要合理组织着色器中变量的问题。这是我们提到的第一个问题。


# UBO的概念
本节开始提到的第二个问题是，如何在多个着色器之间简洁的共享变量。GLSL中可以通过uniform buffer来实现。uniform buffer的实现思路为： 在多个着色器中定义相同的uniform block(就是上面的interface block,使用uniform限定符定义），然后将这些uniform block绑定到对应的uniform buffer object，而uniform buffer object中实际存储这些需要共享的变量。着色器中的uniform block和主程序中的uniform buffer object，是通过OpenGL的绑定点(binding points)连接起来的，它们的关系如下图所示(来自[www.learningopengl.com Advanced GLSL](http://www.learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL))：

![uniform buffer](http://img.blog.csdn.net/20161001165152008)

使用时，每个shader中定义的uniform block有一个索引，通过这个索引连接到OpenGL的绑定点x；而主程序中创建uniform buffer object，传递数据后，将这个UBO绑定到对应的x，此后shader中的uniform block就和OpenGL中的UBO联系起来，我们在程序中操作UBO的数据，就能够在不同着色器之间共享了。例如上图中，着色器A和B定义的Matrices的索引都指向绑定点0，他们共享openGL的uboMatrices这个UBO的数据。同时着色器A的Lights和着色器B的Data，分别指向不同的UBO。

# UBO的使用
在上面我们介绍了UBO的概念，下面通过实例了解UBO的实际使用。UBO的实现依赖于着色器中uniform block的定义，uniform block的内存布局四种形式:shared​, packed​, std140​, and std430​(GLSL4.3以上支持)，默认是shared内存布局。本节我们重点学习shared和std140这两种内存布局形式，其他的形式可以在需要时自行参考[OpenGL规范](https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt)。

- shared 默认的内存布局 采用依赖于具体实现的优化方案，但是保证在不同程序中具有相同定义的block拥有相同的布局，因此可以在不同程序之间共享。要使block能够共享必须注意block具有相同定义，同时所有成员显式指定数组的大小。同时shared保证所有成员都是激活状态，没有变量被优化掉。

- std140 这种方式明确的指定alignment的大小，会在block中添加额外的字节来保证字节对齐，因而可以提前就计算出布局中每个变量的位移偏量，并且能够在shader之间共享；不足在于添加了额外的padding字节。**稍后会介绍字节对齐和padding相关内容**。

下面通过两个简单例子，来熟悉std140和默认的shared内存布局。这个例子将会在屏幕上通过4个着色器绘制4个不同颜色的立方体，在着色器之间共享的是投影矩阵和视变换矩阵，以及为了演示shared layout而添加的混合颜色的示例。

# layout std140

## 字节对齐的概念
字节对齐的一个经典案例就是C语言中的结构体变量，例如下面的结构体:
```cpp
struct StructExample {
	char c;  
	int i;  
	short s; 
}; 
```

你估计它占用内存大小多少字节？ 假设在int 占用4字节，short占用2个字节，那么整体大小等于 1+ 4+ 2 = 7字节吗？

答案是否定的。在Windows平台测试，当int占用4个字节，short占用2个字节是，实际占用大小为12个字节，这12个字节是怎么算出来的呢？  就是用到了字节补齐的概念。实际上上述结构体的内存布局为：

```cpp
   struct StructExample {
	char c;  // 0 bytes offset, 3 bytes padding
	int i;   // 4 bytes offset
	short s; // 8 bytes offset, 2 bytes padding
}; // End of 12 bytes
```
内存布局如下图所示：
![内存布局](http://img.blog.csdn.net/20161001182849349)

字节对齐的一个重要原因是为了使机器访问更迅速。例如在32字长的地址的机器中，每次读取4个字节数据，所以将字节对齐到上述地址 0x0000,0x0004和0x0008, 0x000C将使读取更加迅速。否则例如上面结构体中的int i将跨越两个字长(0x0000和0x0004)，需要两次读取操作，影响效率。当然关于为什么使用字节对齐的更详细分析，感兴趣地可以参考[SO Purpose of memory alignment](http://stackoverflow.com/questions/381244/purpose-of-memory-alignment)。

关于字节对齐，我们需要知道的几个要点就是(参考自[wiki Data structure alignment](https://en.wikipedia.org/wiki/Data_structure_alignment))：

- 一个内存地址，当它是n字节的倍数时，称之为n字节对齐，这里n字节是2的整数幂。

- 每种数据类型都有它自己的字节对齐要求(alignment)，例如char是1字节，int一般为4字节，float为4字节对齐，8字节的long则是8字节对齐。

- 当变量的字节没有对齐时，将额外填充字节(padding)来使之对齐。

上面的结构体中，int变量i需要4字节对齐，因此在char后面填充了3个字节，同时结构体变量整体大小需要满足最长alignment成员的字节对齐，因此在short后面补充了2个字节，总计达到12字节。

关于字节对齐这个概念，介绍到这里，希望了解更多地可以参考[The Lost Art of C Structure Packing](www.catb.org/esr/structure-packing/)。

## std140的字节对齐
std140内存布局同样存在字节对齐的概念，你可以参考[官方文档](https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt)获取完整描述。常用标量int,float，bool要求4字节对齐，4字节也被作为一个基础值N，这里列举几个常用的结构的字节对齐要求：

|类型| 对齐基数(base alignment)|
|--------|---------|
|标量，例如 int  bool| 每个标量对齐基数为N |
|vector	|2N 或者 4N， vec3的基数为4N.|
|标量或者vector的数组| 每个元素的基数等于vec4的基数.|
|矩阵|	以列向量存储, 列向量基数等于vec4的基数.|
|结构体 |元素按之前规则，同时整体大小填充为vec4的对齐基数|

例如一个复杂的uniform block定义为:

```glsl
   layout (std140) uniform ExampleBlock
{
    //               // base alignment  // aligned offset
    float value;     // 4               // 0 
    vec3 vector;     // 16              // 16  (must be multiple of 16 so 4->16)
    mat4 matrix;     // 16              // 32  (column 0)
                     // 16              // 48  (column 1)
                     // 16              // 64  (column 2)
                     // 16              // 80  (column 3)
    float values[3]; // 16              // 96  (values[0])
                     // 16              // 112 (values[1])
                     // 16              // 128 (values[2])
    bool boolean;    // 4               // 144
    int integer;     // 4               // 148
}; 
```
上面的注释给出了它的字节对齐，其中填充了不少字节，可以根据上面表中给定的对齐基数提前计算出来，在主程序中可以设置这个UBO的变量:

```cpp
   GLuint exampleUBOId;
	glGenBuffers(1, &exampleUBOId);
	glBindBuffer(GL_UNIFORM_BUFFER, exampleUBOId);
	glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_DYNAMIC_DRAW); // 预分配空间 大小可以提前根据alignment计算
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, exampleUBOId); // 绑定点为1
	// step4 只更新一部分值
	glBindBuffer(GL_UNIFORM_BUFFER, exampleUBOId);
	GLint b = true; // 布尔变量在GLSL中用4字节表示 因此这里用int存储
	glBufferSubData(GL_UNIFORM_BUFFER, 144, 4, &b); // offset可以根据UBO中alignment提前计算
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
```

> **说明:** 上面最终计算出的大小为152，UBO整体不必满足vec4的字节对齐要求。152 /4 = 38，满足N的对齐要求即可。

从上面可以看到，当成员变量较多时，这种手动计算offset的方法比较笨拙，可以事先编写一个自动计算的函数库，以减轻工作负担。

## std140的简单例子
下面通过一个简单例子来熟悉UBO的使用。

**Step1:** 首先我们在顶点着色器中定义uniform block如下:

```glsl
   #version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;


uniform mat4 model; // 因为模型变换矩阵一般不能共享 所以单独列出来

// 定义UBO
layout (std140) uniform Matrices
{
   mat4 projection;
   mat4 view;
};  // 这里没有定义instance name，则在使用时不需要指定instance name


void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
}
```

**Step2** 在主程序中设置着色器的uniform block索引指向到绑定点0：

```cpp
   // step1 获取shader中 uniform buffer 的索引
	GLuint redShaderIndex = glGetUniformBlockIndex(redShader.programId, "Matrices");
	GLuint greeShaderIndex = glGetUniformBlockIndex(greenShader.programId, "Matrices");
	...
	// step2 设置shader中 uniform buffer 的索引到指定绑定点
	glUniformBlockBinding(redShader.programId, redShaderIndex, 0); // 绑定点为0
	glUniformBlockBinding(greenShader.programId, greeShaderIndex, 0);
	...
```
这里为了演示代码中重复写出了4个着色器，实际中可以通过vector装入这4个着色器简化代码。

**Step3: **  创建UBO，并绑定到绑定点0
我们需要传入2个mat4矩阵，由于mat4中每列的vec4对齐，因此两个mat4中没有额外的padding，大小即为2*sizeof(mat4)。

```cpp
   GLuint UBOId;
	glGenBuffers(1, &UBOId);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOId);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW); // 预分配空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOId, 0, 2 * sizeof(glm::mat4)); // 绑定点为0
```
**Step4: ** 更新UBO中的数据
这里使用前面介绍的[glBufferSubData](http://blog.csdn.net/wangdingqiaoit/article/details/52662270)更新UBO中数据，例如更新视变换矩阵如下：

```cpp
 glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵
glBindBuffer(GL_UNIFORM_BUFFER, UBOId);
glBufferSubData(GL_UNIFORM_BUFFER, 		sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
```
通过上面的步骤，我们完成了着色器中unifrom block和UBO的连接，实现了投影矩阵和视变换矩阵在4个着色器之间的共享，绘制4个立方体如下图所示：

![layout std140](http://img.blog.csdn.net/20161001173835667)

## 验证ExampleBlock

这里在着色器中添加一段代码测试下上面那个复杂的ExampleBlock的内容，我们在主程序中设置boolean变量为true，在着色器中添加一个判断，如果boolean为true，则输出白色立方体：

```glsl
   if(boolean)
	{
	  color = vec4(1.0, 1.0, 1.0, 1.0);
	}
```
最终显示获得了4个全是白色的立方体，效果如下：
![四个白色立方体](http://img.blog.csdn.net/20161001193232712)

这就验证了上述计算出那个复杂ExampleBlock的大小为152，boolean变量位移偏量为144是正确的。



## layout shared
同std140内存布局方式不一样，shared方式的内存布局依赖于具体实现，因此我们无法提前根据某种字节对齐规范计算出UBO中变量的位移偏量和整体大小，因此在使用shared方式时，我们需要多次利用OpenGL的函数来查询UBO的信息。

这里在着色器中定义一个用于混合颜色的uniform block:
```glsl
#version 330 core
// 使用默认shared​方式的UBO
uniform mixColorSettings {
	vec4  anotherColor;
	float mixValue;
};
out vec4 color;
void main()
{
	color = mix(vec4(0.0, 0.0, 1.0, 1.0), anotherColor, mixValue);
}
```

在出程序中首先查询UBO整体大小，预分配空间：

```cpp
GLuint colorUBOId;
glGenBuffers(1, &colorUBOId);
glBindBuffer(GL_UNIFORM_BUFFER, colorUBOId);
// 获取UBO大小 因为定义相同 只需要在一个shader中获取大小即可
GLint blockSize;
glGetActiveUniformBlockiv(redShader.programId, redShaderIndex,
	GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
glBufferData(GL_UNIFORM_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW); // 预分配空间
glBindBuffer(GL_UNIFORM_BUFFER, 0);
glBindBufferBase(GL_UNIFORM_BUFFER, 1, colorUBOId); // 绑定点为1
```
然后，通过查询UBO中成员变量的索引和位移偏量来设置变量值：

```cpp
   // 通过查询获取uniform buffer中各个变量的索引和位移偏量
const GLchar* names[] = {
	"anotherColor", "mixValue"
};
GLuint indices[2];
glGetUniformIndices(redShader.programId, 2, names, indices);
GLint offset[2];
glGetActiveUniformsiv(redShader.programId, 2, indices, GL_UNIFORM_OFFSET, offset);
// 使用获取的位移偏量更新数据
glm::vec4 anotherColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
GLfloat mixValue = 0.5f;
glBindBuffer(GL_UNIFORM_BUFFER, colorUBOId);
glBufferSubData(GL_UNIFORM_BUFFER, offset[0], sizeof(glm::vec4), glm::value_ptr(anotherColor));
glBufferSubData(GL_UNIFORM_BUFFER, offset[1], sizeof(glm::vec4), &mixValue);
glBindBuffer(GL_UNIFORM_BUFFER, 0);
```

和上面std140定义的uniform block一起工作，产生的混合颜色效果如下图所示：

![混合颜色](http://img.blog.csdn.net/20161001191127511)

从上面可以看到，使用shared布局时，当变量较多时，这种查询成员变量索引和位移偏量的工作显得比较麻烦。


# 最后的说明

本节学习了interface block概念，以及UBO的两种内存布局方式。限于本节内容较多，部分函数的具体使用未在此展开介绍，需要的可以自行[参考OpenGL文档](https://www.opengl.org/sdk/docs/man/html/glGetActiveUniformsiv.xhtml)。同时本文中关于那个复杂的std140布局的UBO的offset的计算方法，以及使用shared方式时通过查询获取UBO整体大小、索引和偏移量的方法，需要尽量掌握。