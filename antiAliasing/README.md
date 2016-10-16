写在前面
    目前，我们绘制的图形中存在瑕疵的，观察下面这个立方体：
    
  ![走样的立方体](http://img.blog.csdn.net/20161016131522082)
    仔细看，立方体的边缘部分存在折线，如果我们放大了看，则可以看到这种瑕疵更明显：

![边缘走样](http://img.blog.csdn.net/20161016131645692)

这种绘制的物体边缘部分出现锯齿的现象称之为走样(aliasing)。反走样(Anti-aliasing)是减轻这种现象的方法。反走样本身也是一个比较复杂的主题，深入了解需要有信号处理中的背景知识，例如信号采样、信号重构、滤波等知识，本节作为一个初步探讨，我们不深入反走样算法的细节。主要以OpenGL中实现的MSAA(Multi-Sampled Anti-Aliasing)为重点，介绍在OpenGL中反走样的处理。本节示例代码均可以[在我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/antiAliasing)。

> 本节内容整理自：
   1.[www.learnopengl.com](http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing)
   2.[3D Graphics with OpenGL Basic Theory](https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_BasicsTheory.html)

通过本节可以了解到：

- 走样和反走样的基本概念
- 多采样的反走样方法
- 使用多采样的FBO
- 使用多采样FBO的纹理


# 走样和反走样
我们要绘制的图形在理论上是连续的，例如直线，但是屏幕显示设备却是由像素组成的离散形式，当在二维屏幕上显示连续的对象时，总是会出现失真现象，这称之为走样(aliasing)。用于减轻或者消除这种图形失真现象的技术称之为反走样(Anti-aliasing)。例如下面的直线(图中a，来自[Rasterization lecture](https://web.cs.wpi.edu/~emmanuel/courses/cs543/f13/slides/lecture10_p3.pdf))，当采样某种直线绘制算法显示到屏幕设备上时就产生了走样现象(图中b)：

![line-jaggy](http://img.blog.csdn.net/20161016133404544)

要比较好地理解这个现象，我们有必要了解下一些基础知识。

## 像素和分辨率

像素(pixel, 是picture element的简称)是图形的最小表示单位，一个图形的大小通常用包含的像素个数来表示，如下图所示：

![pixel](http://img.blog.csdn.net/20161016134127252)

像素所在的坐标系是屏幕坐标系，通常用像素所在的行列来表示，例如下图(来自[Pixel Coordinates](http://math.hws.edu/graphicsbook/c2/s1.html))：

![pixel-coordinates](http://img.blog.csdn.net/20161016134256298)

上图中左右都是12x8大小的像素块，左边一个的行从上到下计算，右边一个的行从下往上计算，图中位置(3,5)所表示的像素有区别。像素一般在示例时用点或者方块表示说明问题，实际上像素既不是点，也不是方块，它是不可见的没有区域大小的采样点。(可参考[Sampling, Aliasing, & Mipmaps](http://www.cs.rpi.edu/~cutler/classes/advancedgraphics/F05/lectures/20_sampling_aliasing.pdf))。像素的值，表示的是亮度或者颜色的强度，例如常见的8位颜色图像，其中每个像素包含RGB颜色。

分辨率（resolution）是显式设备的参数，一般用包含的总像素个数说明，用列数(对应显示器宽)x行数(对应显示器高)来说明，例如1024 x768，这里宽高比(aspect ratio)为4:3。

## 光栅化(raterization)
光栅化是将输入的图元转换为屏幕坐标对齐的片元(fragment)的过程。我们看下图的渲染管线(来自[3D Graphics with OpenGLBasic Theory](https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_BasicsTheory.html))：
![渲染管线](http://img.blog.csdn.net/20161016141101450)

这里首先处理的是输入的顶点以及由gl命令指定绘制的图元类型(点、直线、三角形等)，这一阶段由顶点着色器完成，顶点着色器输出经过坐标变换后的顶点。下一阶段光栅处理，运用光栅扫描转换算法（scan-conversion）将输入的图元转换为片元（fragment）。下一阶段，片元着色器处理输入的片元，决定了最终屏幕上每个像素的颜色。这里需要注意的是，片元一般也可以对应像素，但是当多采样时一个像素的颜色可能是多个片元共同决定的。像素是二维的(x,y)，片元却是三维的(x,y,z)，其中z表示深度坐标，用于进行深度测试。

在这个流水线中，光栅化的过程就是处理输入的图元，并最终决定产生哪些片元的过程，如下图所示(来自[Graphics and Rendering](http://alfonse.bitbucket.org/oldtut/Basics/Intro%20Graphics%20and%20Rendering.html))：

![光栅扫描](http://img.blog.csdn.net/20161016143004396)



有了上面的基础后，我们可以明白，当把一条连续的直线，通过光栅扫描算法(例如Bresenham算法，具体可以参考[直线段的扫描转换算法](http://netclass.csu.edu.cn/NCourse/hep089/Chapter2/CG_Txt_2_006.htm))转换为片元时，由于直线时连续的，而片元的坐标对齐屏幕上像素，采样整数坐标，不得不对浮点数坐标取整，导致扫描转换后确定的片元和原先要表达的直线存在差距，这种差距引起了图形的走样。

通过上面，我们对走样有了一个基本概念，当然这只是直观上的理解，如果需要深入了解，还需要从信号处理的角度看问题，在此不再深入，如果感兴趣，可以自行参考[Antialiasing](http://web.cse.ohio-state.edu/~whmin/courses/cse5542-2013-spring/16-antialiasing.pdf)。

下面熟悉两种反走样的方法。

# SuperSampling

超级采样反走样方法(Super-Sampled Anti-Aliasing ,SSAA)，是通过以更高的分辨率来采样图形，然后再显示在低分辨率的设备上，从而减少失真的方法。例如下面的图形表示了增加分辨率后，绘制直线的差别：

![增加分辨率](http://img.blog.csdn.net/20161016144640528)

通过将高分辨率的图像，显示在低分辨率的设备上，确实能有效减轻走样现象，但是存在的弊端就是，要为这些多出来的像素，进行更多的计算，以及内存开销很大。这是一种比较传统的方法。

# MultiSampling
多采样技术(Multi-Sampled Anti-Aliasing, MSAA)是对SSAA的改进，改进之处在于执行片元着色器的次数并没有明显增加，对边缘部分却进行了很好的反走样。多采样相对于单采样，单采样在一个像素上，以像素中心为标准，当光栅化时，如果图元覆盖了这个中心点，那么就产生这个像素对应的片元，否则不影响这个像素；而多采样，则在每个像素上进行细分，分出更多的子采样点(sub-sample)，如下图所示(来自[Anti-Aliasing](http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing))：

![多采样](http://img.blog.csdn.net/20161016145736720)

当图元的一部分覆盖了像素中的子采样点时，则会执行片元着色器，片元着色器的执行不是以子采样点为单位，也就是说不管有多个子采样点，对于每个图元，这个像素只执行一次片元着色器。当颜色缓冲中这些子采样点，填充了所有绘制的图元的颜色时，将取平均值计算最终这个像素的颜色，这个颜色将是唯一值。

例如上面的图中，三角形图元覆盖了2个采样点，那么这个像素的最终颜色由三角形覆盖的2个采样点的颜色和另外两个采样点的颜色(例如这个颜色可能是glClearColor指定的颜色)混合后的均值决定。

下面的图是单采样对应的光栅化过程(来自[Anti-Aliasing](http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing))：

![单采样](http://img.blog.csdn.net/20161016151210983)

下面是对比的多采样光栅化过程(来自[Anti-Aliasing](http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing))：
![多采样](http://img.blog.csdn.net/20161016151250103)

通过多采样，绘制的三角形的边缘部分，因为有了和背景颜色的混合，而减轻了走样现象，如下图所示(来自[Anti-Aliasing](http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing))：

![多采样效果](http://img.blog.csdn.net/20161016151429017)

使用多采样方法时，由于每个像素包含更多的采样点，而这些采样也需要存储颜色、深度值，以及进行模板测试，因而需要更大的颜色缓冲区、深度缓冲区和模板缓冲区。

上面直观上介绍了MSAA方法，实现细节没有探讨，感兴趣地可以自行参考算法实现。在OpenGL中内置了多采样反走样方法，因此下面我们将介绍在OpenGL中的使用。

# 启用多采样
要启用多采样，我们需要创建一个支持多采样的缓冲区(multisample  buffer)，一般由窗口系统为我们生成。我们这里使用GLFW的windowHint来让GLFW库帮我们生成这个多采样缓冲区：

```cpp
  glfwWindowHint(GLFW_SAMPLES, 4);
```
上面第二个参数告诉GLFW创建的多采样中每个像素包含的子采样点个数，一般填写4就好。如果填写太大，将影响图形绘制性能。注意，参数不要写成了：
```cpp
   glfwWindowHint(GL_SAMPLES, 4); // 应该是GLFW参数，错误
```
同时需要确保开启了OpenGL多采样，这个参数默认是开启的:

```cpp
   glEnable(GL_MULTISAMPLE);  
```
通过开启多采样，我们绘制的立方体效果如下：

![反走样的立方体](http://img.blog.csdn.net/20161016153114370)

你可以通过O和F开启和关闭反走样，对比效果。
放大这个反走样的立方体，我们发现边缘部分的锯齿有了明显改善：

![反走样的边缘](http://img.blog.csdn.net/20161016153222206)


# 使用多采样的FBO离屏反走样

上面开启了全局的多采样，如果需要为部分内容开启多采样，可以使用FBO实现离屏渲染的反走样图形，然后将这个离屏渲染的图形赋值到默认的FBO中。在[前面FBO一节](http://blog.csdn.net/wangdingqiaoit/article/details/52423060)已经介绍了FBO的基本使用，如果对这个概念不熟悉，可以回过头去查看。当使用多采样FBO时需要做些调整。

# 创建多采样的纹理附加对象
这里创建一个附加FBO的textures时，附加的纹理不再是GL_TEXTURE_2D，而是GL_TEXTURE_2D_MULTISAMPLE:

```cpp
  static GLuint makeMAAttachmentTexture(GLint samplesNum = 4, GLint internalFormat = GL_RGB,
		GLsizei width = 800, GLsizei height = 600)
{
GLuint textId;
glGenTextures(1, &textId);
// 注意修改target为GL_TEXTURE_2D_MULTISAMPLE
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textId); 
glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samplesNum, internalFormat,width, height, GL_TRUE); 
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
return textId;
}
```
# 创建多采样的RBO
与之前创建RBO不同之处在于预分配内存时，需要指定多采样的子采样点个数，是下面的第二个参数：

```cpp
glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, 
   GL_DEPTH24_STENCIL8,WINDOW_WIDTH, WINDOW_HEIGHT); // 预分配内存
```
创建完了附加纹理和RBO后，我们就构建了一个状态合法的FBO了，完整创建过程如下：

```cpp
   bool prepareMSFBO(const int samplesCnt, GLuint& colorTextId, GLuint& fboId)
{
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	// 附加 color attachment
	colorTextId = TextureHelper::makeMAAttachmentTexture(samplesCnt, GL_RGB); // 创建FBO中的多采样纹理
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTextId, 0);
	// 附加 depth stencil RBO attachment
	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesCnt, GL_DEPTH24_STENCIL8,
		WINDOW_WIDTH, WINDOW_HEIGHT); // 预分配内存
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}
```
有了合法的FBO后，我们分两个阶段绘制场景：

```cpp
	// 第一遍 绘制到多采样的FBO中
	glBindFramebuffer(GL_FRAMEBUFFER, MSFBOId);
	glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 省略绘制场景代码
	// 第二遍 将多采样的FBO纹理复制到默认FBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBOId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // 这里0表示使用默认的FBO
glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 
		0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
```
注意第二遍，复制多采样的FBO到默认FBO时，我们使用了函数[glBlitFramebuffer](https://www.khronos.org/opengles/sdk/docs/man3/html/glBlitFramebuffer.xhtml)，这个函数从读buffer拷贝内容到写buffer，具体参数如下：

> **API** void glBlitFramebuffer( 	GLint srcX0,
  	GLint srcY0,
  	GLint srcX1,
  	GLint srcY1,
  	GLint dstX0,
  	GLint dstY0,
  	GLint dstX1,
  	GLint dstY1,
  	GLbitfield mask,
  	GLenum filter);
  	1.srcX0, srcY0, srcX1, srcY1指定了读取buffer的区域.
  	2.dstX0, dstY0, dstX1, dstY1指定了写入buffer的区域
  	3.mask指定了从读buffer复制到写buffer的内容，允许的值为L_COLOR_BUFFER_BIT,GL_DEPTH_BUFFER_BIT 和GL_STENCIL_BUFFER_BIT.
  	4.当图形被拉伸时的插值选项，允许值为GL_NEAREST or GL_LINEAR.

在使用这个函数之前，我们需要将多采样纹理绑定到读buffer，而默认的FBO置为写buffer。经过两个步骤的绘制，最终得到和上面使用MSAA效果相同的结果，但是这里我们是执行的离屏渲染，而不是全局采用反走样。

# 多采样FBO纹理的后处理
如果需要使用上述多采样的FBO的纹理做一些后处理效果(postProcessing)，直接使用这个多采样的纹理是不行的，因为他和默认的FBO的子采样点个数不相同，无法直接在着色器中采样。这里的技巧是，借助一个中间FBO，分三次绘制：

1.渲染到多采样FBO。
2.多采样FBO渲染到中间FBO，这个纹理可供我们正常在着色器中采样了。
3.在着色器中采样中间FBO的纹理，渲染到一个矩形上，作为场景最终效果。

首先，我么准备多采样FBO和一个只包含颜色附加的中间FBO；然后分三次绘制场景，流程如下：

```cpp
   // 创建多采样的FBO
   GLuint MSTextId, MSFBOId;
   prepareMSFBO(SAMPLES_COUNT, MSTextId, MSFBOId);
   // 创建一个中间FBO
   GLuint screenTextId, intermediateFBOId;
   prepareIntermediateFBO(screenTextId, intermediateFBOId);
   // 第一遍 绘制到多采样的FBO中
  glBindFramebuffer(GL_FRAMEBUFFER, MSFBOId);
  // 第二遍 将多采样的FBO纹理复制到中间FBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBOId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBOId);
glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 
			0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
// 第三遍 利用中间FBO的纹理 绘制到默认FBO中
glBindTexture(GL_TEXTURE_2D, screenTextId);
glDrawArrays(GL_TRIANGLES, 0, 6);
```

使用多采样FBO的纹理，我们经过模糊效果处理的立方体，如下图所示：

![多采样FBO纹理的使用](http://img.blog.csdn.net/20161016160618465)

# 最后的说明
本节初步学习了反走样这个主题，熟悉了OpenGL中使用MSAA方法处理反走样。反走样本身是一个比较复杂的主题，存在众多的反走样方法，而且涉及到信号处理的背景知识，要学习好这个主题需要付出一定的努力。有需要深入了解的可以查看[A Quick Overview of MSAA](https://mynameismjp.wordpress.com/2012/10/24/msaa-overview/)，以及[东南大学反走样课件](http://cose.seu.edu.cn/seugraph/AdminComponents%5CUploadedCourseAcc%5C8_Antialiasing.pdf)。







