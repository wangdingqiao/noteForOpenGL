写在前面
     一直以来，我们在使用OpenGL渲染时，最终的目的地是默认的帧缓冲区，实际上OpenGL也允许我们创建自定义的帧缓冲区。使用自定义的帧缓冲区，可以实现镜面，离屏渲染，以及很酷的后处理效果。本节将学习帧缓存的使用，文中示例代码均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/frameBufferObject)。

> 本节内容整理自
> 1.[OpenGL Frame Buffer Object (FBO)](http://www.songho.ca/opengl/gl_fbo.html)
> 2.[www.learnopengl.com Framebuffers](http://www.learnopengl.com/#!Advanced-OpenGL/Framebuffers)


# FBO概念

在OpenGL中，渲染管线中的顶点、纹理等经过一系列处理后，最终显示在2D屏幕设备上，渲染管线的最终目的地就是帧缓冲区。帧缓冲包括OpenGL使用的颜色缓冲区(color buffer)、深度缓冲区(depth buffer)、模板缓冲区(stencil buffer)等缓冲区。默认的帧缓冲区由窗口系统创建，例如我们一直使用的GLFW库来完成这项任务。这个默认的帧缓冲区，就是目前我们一直使用的绘图命令的作用对象，称之为**窗口系统提供的帧缓冲区**(window-system-provided framebuffer)。

OpenGL也允许我们手动创建一个帧缓冲区，并将渲染结果重定向到这个缓冲区。在创建时允许我们自定义帧缓冲区的一些特性，这个自定义的帧缓冲区，称之为**应用程序帧缓冲区**(application-created framebuffer object )。

同默认的帧缓冲区一样，自定义的帧缓冲区也包含颜色缓冲区、深度和模板缓冲区，这些逻辑上的缓冲区（logical buffers）在FBO中称之为可附加的图像(framebuffer-attachable images)，他们是可以附加到FBO的二维像素数组（2D arrays of pixels ）。

FBO中包含两种类型的附加图像(framebuffer-attachable): 纹理图像和RenderBuffer图像(texture images and renderbuffer images)。附加纹理时OpenGL渲染到这个纹理图像，在着色器中可以访问到这个纹理对象；附加RenderBuffer时，OpenGL执行离屏渲染(offscreen rendering)。

之所以用附加这个词，表达的是FBO可以附加多个缓冲区，而且可以灵活地在缓冲区中切换，一个重要的概念是附加点(attachment points)。FBO中包含一个以上的颜色附加点，但只有一个深度和模板附加点，如下图所示(来自[songho FBO](http://www.songho.ca/opengl/gl_fbo.html))：

![FBO attachment point](http://img.blog.csdn.net/20160903125300592)

一个FBO可以有
(GL_COLOR_ATTACHMENT0,..., GL_COLOR_ATTACHMENTn)
多个附加点，最多的附加点可以通过查询GL_MAX_COLOR_ATTACHMENTS变量获取。

**值得注意的是**：从上面的图中我们可以看到，FBO本身并不包含任何缓冲对象，实际上是通过附加点指向实际的缓冲对象的。这样FBO可以快速地切换缓冲对象。

# 创建FBO 
同OpenGL中创建其他缓冲对象一样，创建和销毁FBO的步骤也很简单：

```cpp
void glGenFramebuffers(GLsizei n, GLuint* ids)
void glDeleteFramebuffers(GLsizei n, const GLuint* ids)
```
创建之后，我们需要将FBO绑定到目标对象：

```cpp
    void glBindFramebuffer(GLenum target, GLuint id) 
```
这里的target一般可以填写GL_FRAMEBUFFER，这个缓冲区将会用来进行读和写操作；如果需要绑定到读操作的缓冲区使用GL_READ_FRAMEBUFFER，支持 glReadPixels这类读操作；如果需要绑定到写操作的缓冲区使用GL_DRAW_FRAMEBUFFER，支持渲染、清除等操作。

OpenGL要求，一个完整的FBO需要满足以下条件(来自[FrameBufffer](http://www.learnopengl.com/#!Advanced-OpenGL/Framebuffers)):

- 至少附加一个缓冲区(颜色、深度或者模板)
- 至少有一个颜色附加
- 所有的附加必须完整(预分配了内存)
- 每个缓冲区的采样数需要一致

关于采样，后面会学习，暂时不做讨论。判断一个FBO是否完整，可以如下:

```cpp
   if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
```
如果FBO不完整将不能正常工作。
那么我们需要按照上述要求构建一个完整的FBO。

# 创建纹理附加图像

创建FBO的附加纹理如同平常使用纹理一样，不同的是，这里只是为纹理预分配空间，而不需要真正的加载纹理，因为当使用FBO渲染时渲染结果将会写入到我们创建的这个纹理上去。附加纹理使用函数[glFramebufferTexture2D](https://www.opengl.org/sdk/docs/man/html/glFramebufferTexture.xhtml)。

> **API** void glFramebufferTexture2D( 	GLenum target,
  	GLenum attachment,
  	GLenum textarget,GLuint texture,GLint level);
> 1.target表示绑定目标，参数可选为GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER, or GL_FRAMEBUFFER。
> 2.attechment表示附加点，可选值为GL_COLOR_ATTACHMENTi, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT or GL_DEPTH_STENCIL_ATTACHMMENT。
>3. textTarget表示纹理的绑定目标，我们使用二维纹理填写GL_TEXTURE_2D即可。
>4. texture表示实际的纹理对象。
>5. level表示 mipmap级别，我们填写0即可。
 
这里的texture是我们实际创建的纹理对象，在创建纹理对象时使用代码：

```cpp
GLuint texture;
glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
```
这里需要注意的是[glTexImage2D](https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml)函数，末尾的NULL表示我们只预分配空间，而不实际加载纹理。glTexImage2D函数也是一个OpenGL中相对复杂的一个函数。

> **API** void glTexImage2D( 	GLenum target,
  	GLint level,
  	GLint internalFormat,
  	GLsizei width,
  	GLsizei height,
  	GLint border,
  	GLenum format,
  	GLenum type,
  	const GLvoid * data);
 
 在前面[二维纹理](http://blog.csdn.net/wangdingqiaoit/article/details/51457675)一节已经介绍过这个函数，这里重点说下创建FBO纹理时需要注意的。函数中后面三个参数format、type、data表示的是内存中图像像素的信息，包括格式，类型和指向内存的指针。而internalFormat表示的是OpenGL内存存储纹理的格式，表示的是纹理中颜色成分的格式。从纹理图片的内存转移到OpenGL内存纹理存储是一个像素转移操作(Pixel Transfer )，关于这个部分的细节比较多，不在这里展开，感兴趣地可以参考[OpenGL wiki-Pixel Transfer ](https://www.opengl.org/wiki/Pixel_Transfer)。
 
上面填写的纹理格式GL_RGB，以及GL_UNSIGNED_BYTE表示纹理包含红绿蓝三色，并且每个成分用无符号字节表示。600,800表示我们分配的纹理大小，注意这个纹理需要和我们渲染的屏幕大小保持一致，如果需要绘制与屏幕不一致的纹理，使用glViewport函数进行调节。

上面创建的纹理图像，可以附加到FBO：

```cpp
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); 
```
这里我们附加到了颜色附加点。在绘制时，如果需要开启深度测试还需要附加一个深度缓冲区，这里我们也附加一个深度-模板到纹理中。将创建纹理的代码封装到[texture.h](https://github.com/wangdingqiao/noteForOpenGL/blob/master/frameBufferObject/renderToTexture/texture.h)中，完整的用纹理图像构建一个FBO的代码如下：

```cpp
/*
* 附加纹理到Color, depth ,stencil Attachment
*/
bool prepareFBO1(GLuint& colorTextId, GLuint& depthStencilTextId, GLuint& fboId)
{
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	// 附加 纹理 color attachment
	colorTextId = TextureHelper::makeAttachmentTexture(0, GL_RGB, WINDOW_WIDTH,WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextId, 0);
	// 附加 depth stencil texture attachment
	depthStencilTextId = TextureHelper::makeAttachmentTexture(0, GL_DEPTH24_STENCIL8,WINDOW_WIDTH, 
	WINDOW_HEIGHT, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_TEXTURE_2D, depthStencilTextId, 0);
	// 检测完整性
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}
```
到此，我们的FBO就满足了基本要求，可以使用了。在利用FBO作图前，我们继续介绍另一个附加图像-RenderBuffer。

# RenderBuffer Object
纹理图像附加到FBO后，执行渲染后，我们可以在后期着色器处理中访问到纹理，这给一些需要多遍处理的操作提供了很大方便。当我们不需要在后期读取纹理时，我们可以使用Renderbuffer这种附加图像，它主要用来存储深度、模板这类没有与之对应的纹理格式的缓冲区。创建和销毁RenderBuffer也很简单，如下:

```cpp
void glGenRenderbuffers(GLsizei n, GLuint* ids)
void glDeleteRenderbuffers(GLsizei n, const Gluint* ids)
```
创建完毕后，仍然需要绑定道目标对象:

```cpp
   glBindRenderbuffer(GL_RENDERBUFFER, rbo);  
```
需要注意的是，我们还需要为RBO预分配内存空间:

```cpp
   void glRenderbufferStorage(GLenum  target,
                           GLenum  internalFormat,
                           GLsizei width,
                           GLsizei height)
```
这个函数为指定内部格式的RBO预分配空间。

当上述步骤完成后，我们可以将RBO绑定到FBO。

上面的纹理图像中使用了纹理作为深度和模板缓冲区，这里我们将深度模板缓冲区使用RBO代替：

```cpp
 /*
* 附加纹理到Color Attachment 
* 同时附加RBO到depth stencil Attachment
*/
bool prepareFBO2(GLuint& textId, GLuint& fboId)
{
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	// 附加纹理 color attachment
	textId = TextureHelper::makeAttachmentTexture(0, GL_RGB, WINDOW_WIDTH,WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE); 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textId, 0);
	// 附加 depth stencil RBO attachment
	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
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
到此，我们也利用RBO创建了一个完整的FBO。


# 绘制到纹理

上面利用纹理和RBO创建的FBO，我们在OpenGL中可以用来将场景绘制到纹理中。首先绑定自定义的FBO执行渲染，然后绑定到默认FBO，我们绘制一个矩形，矩形使用FBO中的纹理填充，得到效果如下图所示：

![FBO render to texture](http://img.blog.csdn.net/20160903140332814)

采用线框模式绘制：

```cpp
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
```
显示的就是一个矩形:

![FBO render to texture wireframe](http://img.blog.csdn.net/20160903140601941)

从上面结果我们可以看到，利用FBO将场景绘制到纹理，在后期绘制矩形时使用这个纹理。这种方式可以制作镜子等效果，十分有用。

# 使用后处理效果(postprocessing)

上面场景绘制到纹理后，我们可以通过操作这个纹理图像，而得到很酷的后处理效果。例如在着色器中，将纹理的颜色进行反转：

```cpp
vec3 inversion()  // 反色
{
  return vec3(1.0 - texture(text, TextCoord));
}
```
得到的效果如下图所示：

![color inversion](http://img.blog.csdn.net/20160903142232244)

后处理也可以采取图像处理的方式，例如使用kernel矩阵。kernel矩阵一般取为3x3矩阵，这个矩阵的和一般为1。通过kernel矩阵，将当前纹理坐标处的纹理扩展到周围9个坐标处的纹理，然后通过权重计算出最终纹理的像素。例如产生浮雕效果的kernel矩阵如下所示：

$$kernel = \begin{bmatrix} -2 & -1 & 0 \\ 
-1 & 1 & 1 \\
0 & 1 & 2
\end{bmatrix}$$

在着色器中，我们定义当前纹理位置的9个周围位置如下：

```cpp
   const float offset = 1.0 / 300;  // 9个位置的纹理坐标偏移量
   // 确定9个位置的偏移量
   vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left 左上方
        vec2(0.0f,    offset),  // top-center 正上方
        vec2(offset,  offset),  // top-right 右上方
        vec2(-offset, 0.0f),    // center-left 中间左边
        vec2(0.0f,    0.0f),    // center-center 正中位置
        vec2(offset,  0.0f),    // center-right 中间右边
        vec2(-offset, -offset), // bottom-left 底部左边
        vec2(0.0f,    -offset), // bottom-center 底部中间
        vec2(offset,  -offset)  // bottom-right  底部右边
    );
```

然后使用kernel矩阵中的权系数，计算最终的纹理像素：

```cpp
   // 计算9个位置的纹理
	vec3 sampleText[9];
	for(int i=0; i < 9;++i)
	{
		sampleText[i] = vec3(texture(text, TextCoord.st + offsets[i]));
	}
	// 利用权值求最终纹理颜色
	vec3 result = vec3(0.0);
	for(int i=0; i < 9;++i)
	{
		result += sampleText[i] * kernel[i];
	}
```
指定不同的kernel将会得到不同的效果，例如指定模糊矩阵，得到模糊的效果如下图所示：

![blur kernel](http://img.blog.csdn.net/20160903141631814)

指定edge-detection矩阵，得到效果如下图所示:

![edge-detection kernel](http://img.blog.csdn.net/20160903141802331)

当着色器中计算纹理坐标的偏移量offset不同时，效果会有所改变。想查看更多的kernel效果，可以访问在线网站[Image Kernels](http://setosa.io/ev/image-kernels/)。


# 最后的说明
本节介绍了FBO的概念和使用，还有一些操作例如FBO的读写、复制操作没有介绍到，同时glTextImage2D这个函数中纹理的内部格式以及内存中像素的格式和类型的说明将是一个比较繁琐的工作，这些内容留到后续学习。关于选择附加纹理还是附加RBO，可以参考[Difference between Frame buffer object, Render buffer object and texture?](http://stackoverflow.com/questions/10117409/difference-between-frame-buffer-object-render-buffer-object-and-texture)。

在附加深度和模板的纹理时（即代码中我们使用depthStencilTextId而不是colorTextId绘制最终的结果），如果我们使用深度和模板的纹理绘图将会得到如下效果：

![depth stencil texture](http://img.blog.csdn.net/20160903142816034)

这个图中主要呈现红色，我分析是因为图中离观察者较远的距离时深度值基本为1，那么取得的纹理颜色基本上就是(1.0,0.0,0.0,1.0)，因而呈现红色；而离观察者近一些的地方，深度值基本上为0，则取得的纹理颜色就是(0.0, 0.0, 0.0, 1.0)，因而呈现出黑色。如果观察者靠近场景中的立方体，那么得到的图像将主要呈现黑色：

![depth stencil texture2](http://img.blog.csdn.net/20160903143116327)

附加的GL_DEPTH24_STENCIL8纹理，底层如何解释为采样后的颜色值，还需要进一步学习和说明。


# 参考资料

1.[OpenGL wiki Image Format](https://www.opengl.org/wiki/Image_Format)
2.[OpenGL wiki Framebuffer Object](https://www.opengl.org/wiki/Framebuffer_Object)
3.[OpenGL wiki Pixel Transfer](https://www.opengl.org/wiki/Pixel_Transfer)
4.[Wiki Framebuffer object](https://en.wikipedia.org/wiki/Framebuffer_object)