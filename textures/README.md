写在前面
    前面两节介绍了[向量和矩阵](http://blog.csdn.net/wangdingqiaoit/article/details/51383052)，以及[坐标和转换](http://blog.csdn.net/wangdingqiaoit/article/details/51394238)相关的数学，再继续讨论模型变换等其他包含数学内容的部分之前，本节介绍二维纹理映射，为后面学习做一个准备。纹理映射本身也是比较大的主题，本节只限于讨论二维纹理的基本使用，对于纹理映射的其他方法，后面会继续学习。可以从[我的github](https://github.com/wangdingqiao/noteForOpenGL/tree/master/textures)下载本节代码。

通过本节可以了解到

- 纹理映射的概念和原理
- 二维纹理映射的处理方法


## 使用纹理增加物体表面细节
要使渲染的物体更加逼真，一方面我们可以使用更多的三角形来建模，通过复杂的模型来逼近物体，但是这种方法会增加绘制流水线的负荷，而且很多情况下不是很方便的。使用纹理，将物体表面的细节映射到建模好的物体表面，这样不仅能使渲染的模型表面细节更丰富，而且比较方便高效。纹理映射就是这样一种方法，在程序中通过为物体指定纹理坐标，通过纹理坐标获取纹理对象中的纹理，最终显示在屏幕区域上，已达到更加逼真的效果。

## 纹素(texel)和纹理坐标
使用纹素这个术语，而不是像素来表示纹理对象中的显示元素，主要是为了强调纹理对象的应用方式。纹理对象通常是通过纹理图片读取到的，这个数据保存到一个二维数组中，这个数组中的元素称为纹素(texel)，纹素包含颜色值和alpha值。纹理对象的大小的宽度和高度应该为2的整数幂，例如16, 32, 64, 128, 256。要想获取纹理对象中的纹素，需要使用纹理坐标(texture coordinate)指定。

纹理坐标应该与纹理对象大小无关，这样指定的纹理坐标当纹理对象大小变更时，依然能够工作，比如从256x256大小的纹理，换到512x256时，纹理坐标依然能够工作。因此纹理坐标使用规范化的值，大小范围为[0,1]，纹理坐标使用uv表示，如下图所示(来自:[Basic Texture Mapping](http://ogldev.atspace.co.uk/www/tutorial16/tutorial16.html)):
![纹理坐标](http://img.blog.csdn.net/20140924154511855)

u轴从左至右，v轴从底向上指向。右上角为(1,1)，左下角为(0,0)。
通过指定纹理坐标，可以映射到纹素。例如一个256x256大小的二维纹理，坐标(0.5,1.0)对应的纹素即是(128,256)。(256x0.5 = 128, 256x1.0 = 256)。

纹理映射时只需要为物体的顶点指定纹理坐标即可，其余部分由片元着色器插值完成，如下图所示(来自[A textured cube](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/))：
![纹理坐标使用](http://img.blog.csdn.net/20160519221437536)

## 模型变换和纹理坐标
所谓模型变换，就是对物体进行缩放、旋转、平移等操作，后面会着重介绍。当对物体进行这些操作时，顶点对应的纹理坐标不会进行改变，通过插值后，物体的纹理也像紧跟着物体发生了变化一样。如下图所示为变换前物体的纹理坐标(来自:[Basic Texture Mapping](http://ogldev.atspace.co.uk/www/tutorial16/tutorial16.html)):
![模型变换前](http://img.blog.csdn.net/20160519221839885)

经过旋转等变换后，物体和对应的纹理坐标如下图所示，可以看出上面图中纹理部分的房子也跟着发生了旋转。(来自:[Basic Texture Mapping](http://ogldev.atspace.co.uk/www/tutorial16/tutorial16.html)):
![模型变换后](http://img.blog.csdn.net/20160519221945825)

**注意** 有一些技术可以使纹理坐标有控制地发生改变，本节不深入讨论，这里我们的纹理坐标在模型变换下保持不变。

## 创建纹理对象
创建纹理对象的过程同前面讲述的创建VBO，VAO类似：
```C++
   GLuint textureId;
   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
```
这里我们绑定到GL_TEXTURE_2D目标，表示二维纹理。

## WRAP参数
上面提到纹理坐标(0.5, 1.0)到纹素的映射，恰好为(128,256)。如果纹理坐标超出[0,0]到[1,1]的范围该怎么处理呢？ 这个就是wrap参数由来，它使用以下方式来处理：

- GL_REPEAT:坐标的整数部分被忽略，重复纹理，**这是OpenGL纹理默认的处理方式**.
- GL_MIRRORED_REPEAT: 纹理也会被重复，但是当纹理坐标的整数部分是奇数时会使用镜像重复。
- GL_CLAMP_TO_EDGE: 坐标会被截断到[0,1]之间。结果是坐标值大的被截断到纹理的边缘部分，形成了一个拉伸的边缘(stretched edge pattern)。
- GL_CLAMP_TO_BORDER: 不在[0,1]范围内的纹理坐标会使用用户指定的边缘颜色。

当纹理坐标超出[0,1]范围后，使用不同的选项，输出的效果如下图所示(来自[Textures objects and parameters](https://open.gl/textures))：

![这里写图片描述](http://img.blog.csdn.net/20160520093932866)

在OpenGL中设置wrap参数方式如下：
```c++
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
```
上面的几个选项对应的都是整数，因此使用glTexParameteri来设置。

## Filter参数
当使用纹理坐标映射到纹素数组时，正好得到对应纹素的中心位置的情况，很少出现。例如上面的(0.5,1.0)对应纹素(128,256)的情况是比较少的。**如果纹理坐标映射到纹素位置(152.34,745.14)该怎么办呢 ?** 

一种方式是对这个坐标进行取整，使用最佳逼近点来获取纹素，这种方式即点采样(point sampling)，也就是最近邻滤波( nearest neighbor filtering)。这种方式容易导致走样误差，明显有像素块的感觉。最近邻滤波方法的示意图如下所示(来自[A Textured Cube](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/))：
![最近邻滤波](http://img.blog.csdn.net/20160520103737303)
图中目标纹素位置，离红色这个纹素最近，因此选择红色作为最终输出纹素。

另外还存在其他滤波方法，例如线性滤波方法(linear filtering)，它使用纹素位置(152.34,745.14)附近的一组纹素的加权平均值来确定最终的纹素值。例如使用 ( (152,745), (153,745), (152,744) and (153,744) )这四个纹素值的加权平均值。权系数通过与目标点(152.34,745.14)的距离远近反映，距离(152.34,745.14)越近，权系数越大，即对最终的纹素值影响越大。线性滤波的示意图如下图所示(来自[A Textured Cube](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/))：
![线性滤波](http://img.blog.csdn.net/20160520103903789)
图中目标纹素位置周围的4个纹素通过加权平均计算出最终输出纹素。

还存在其他的滤波方式，如三线性滤波(Trilinear filtering)等，感兴趣的可以参考[texture filtering wiki](https://en.wikipedia.org/wiki/Texture_filtering)。最近邻滤波和线性滤波的对比效果如下图所示(来自[Textures objects and parameters](https://open.gl/textures))：

![最近邻和线性滤波对比](http://img.blog.csdn.net/20160520104513209)

可以看出最近邻方法获取的纹素看起来有明显的像素块，而线性滤波方法获取的纹素看起来比较平滑。两种方法各自有不同的应用场合，不能说线性滤波一定比最近邻滤波方法好，例如要制造8位图形效果(8 bit graphics,每个像素使用8位字节表示)需要使用最近邻滤波。作为一个兴趣了解，8位图形效果看起来也是很酷的（可以查看[Welcome 8-bit, Pixel-Art Images Gallery!](http://8bitdecals.com/8-bit-image-gallery/))获得更多8位图形），例如下面这张使用Excel制作的8位图(来自[Excel is a great for making 8 bit graphics!](http://imgur.com/gallery/gO8ueXc))：
![这里写图片描述](http://img.blog.csdn.net/20160520105811908)

另外一个问题是，纹理应用到物体上，最终要绘制在显示设备上，这里存在一个纹素到像素的转换问题。有三种情形（参考自[An Introduction to Texture Filtering](http://www.learnopengles.com/android-lesson-six-an-introduction-to-texture-filtering/)）：

-  一个纹素最终对应屏幕上的多个像素  这称之为放大(magnification)
-  一个纹素对应屏幕上的一个像素 这种情况不需要滤波方法
-  一个纹素对应少于一个像素，或者说多个纹素对应屏幕上的一个像素 这个称之为缩小(minification)
放大和缩小的示意图如下：
![magnification and minification](http://img.blog.csdn.net/20140924162515554)

在OpenGL中通过使用下面的函数，为纹理的放大和缩小滤波设置相关的控制选项：
```C++
glTexParameteri(GL_TEXTURE_2D, 
	GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, 
	GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
```
其中GL_LINEAR对应线性滤波，GL_NEAREST对应最近邻滤波方式。

## 使用Mipmaps
考虑一个情景：当物体在场景中离观察者很远，最终只用一个屏幕像素来显示时，这个像素该如何通过纹素确定呢？如果使用最近邻滤波来获取这个纹素，那么显示效果并不理想。需要使用纹素的均值来反映物体在场景中离我们很远这个效果，对于一个 256×256的纹理，计算平均值是一个耗时工作，不能实时计算，因此可以通过**提前计算**一组这样的纹理用来满足这种需求。这组提前计算的按比例缩小的纹理就是Mipmaps。Mipmaps纹理大小每级是前一等级的一半，按大小递减顺序排列为：

 - 原始纹理  256×256 
 - Mip 1 = 128×128
 - Mip 2 = 64×64
 - Mip 3 = 32×32
 - Mip 4 = 16×16
 - Mip 5 = 8×8
 - Mip 6 = 4×4
 - Mip 7 = 2×2
 - Mip 8 = 1×1

OpenGL会根据物体离观察者的距离选择使用合适大小的Mipmap纹理。Mipmap纹理示意图如下所示（来自[wiki Mipmap](https://en.wikipedia.org/wiki/Mipmap)）：
![Mipmap](http://img.blog.csdn.net/20160520121350134)
OpenGL中通过函数glGenerateMipmap(GL_TEXTURE_2D);来生成Mipmap，前提是已经指定了原始纹理。原始纹理必须自己通过读取纹理图片来加载，这个后面会介绍。
如果直接在不同等级的MipMap之间切换，会形成明显的边缘，因此对于Mipmap也可以同纹素一样使用滤波方法在不同等级的Mipmap之间滤波。要在不同等级的MipMap之间滤波，需要将之前设置的GL_TEXTURE_MIN_FILTER选项更改为以下选项之一：

- GL_NEAREST_MIPMAP_NEAREST:  使用最接近像素大小的Mipmap，纹理内部使用最近邻滤波。
- GL_LINEAR_MIPMAP_NEAREST: 使用最接近像素大小的Mipmap，纹理内部使用线性滤波。
- GL_NEAREST_MIPMAP_LINEAR: 在两个最接近像素大小的Mipmap中做线性插值，纹理内部使用最近邻滤波。
- GL_LINEAR_MIPMAP_LINEAR: 在两个最接近像素大小的Mipmap中做线性插值，纹理内部使用线性滤波。

**Mipmap使用注意** 使用使用glGenerateMipmap(GL_TEXTURE_2D)产生Mipmap的前提是你已经加载了原始的纹理对象。使用MipMap时设置GL_TEXTURE_MIN_FILTER选项才能起作用，设置GL_TEXTURE_MAG_FILTER的Mipmap选项将会导致无效操作，OpenGL错误码为GL_INVALID_ENUM。

设置Mipmap选项如下代码所示:
```C++
   glTexParameteri(GL_TEXTURE_2D, 
	   GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
```

## 加载原始纹理
从图片加载纹理这部分工作不是OpenGL函数完成的，可以通过外部库实现。这里我们使用[SOIL](http://www.lonesock.net/soil.html)（Simple OpenGL Image Library）库完成。下载完这个库后，你需要编译到本地平台对应版本。你可以从[我的github处](https://github.com/wangdingqiao/noteForOpenGL/tree/master/libraries)下载已经编译好的32位库。
使用SOIL加载纹理的代码如下：
```C++
GLubyte *imageData = NULL;
int picWidth, picHeight;
imageData = SOIL_load_image("wood.png", 
	&picWidth, &picHeight, 0, SOIL_LOAD_RGB); // 读取图片数据
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
	picWidth,picHeight, 0, GL_RGB, 
	GL_UNSIGNED_BYTE, imageData); // 定义纹理图像
```
其中glTexImage2D函数定义纹理图像的格式，宽度和高度等信息，具体参数如下：

> **API** void [glTexImage2D](https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml)( 	GLenum target,
  	GLint level,
  	GLint internalFormat,
  	GLsizei width,
  	GLsizei height,
  	GLint border,
  	GLenum format,
  	GLenum type,
  	const GLvoid * data);

1.target参数指定设置的纹理目标，必须是GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D等参数。
2.level指定纹理等级，0代表原始纹理，其余等级对应Mipmap纹理等级。
3.internalFormat指定OpenGL存储纹理的格式，我们读取的图片格式包含RGB颜色，因此这里也是用RGB颜色。
4.width和height参数指定存储的纹理大小，我们之前利用SOIL读取图片时已经获取了图片大小，这里直接使用即可。
5. border 参数为历史遗留参数，只能设置为0.
6. 最后三个参数指定原始图片数据的格式(format)和数据类型(type,为GL_UNSIGNED_BYTE, GL_BYTE等值)，以及数据的内存地址(data指针)。

##使用纹理的完整过程
**Step1** 首先要指定纹理坐标，这个坐标和顶点位置、顶点颜色一样处理，使用索引绘制，代码如下所示:
```C++
   // 指定顶点属性数据 顶点位置 颜色 纹理
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f, 0.0f,  // 0
		0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f,  // 1
		0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f,1.0f, 1.0f,  // 2
		-0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f,0.0f, 1.0f   // 3
	};
	GLushort indices[] = {
		0, 1, 2,  // 第一个三角形
		0, 2, 3   // 第二个三角形
	};
```
同顶点位置和颜色一样，需要指定纹理坐标的解析方式。上面的数据格式如下图所示(来自[www.learnopengl.com](http://www.learnopengl.com/#!Getting-started/Textures)):
![data format](http://img.blog.csdn.net/20160520131108489)

这个格式的说明在[OpenGL学习脚印: 绘制一个三角形 ](http://blog.csdn.net/wangdingqiaoit/article/details/51318793)已经讲过，如果不清楚，可以回过头去查看。通过查看上图，我们按照如下方式设置glVertexAttribPointer，让OpenGL知道如何解析上述数据：

```C++
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
```
对应的顶点着色器如下：
```C++
#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 textCoord; // 纹理坐标

out vec3 VertColor;
out vec2 TextCoord;

void main()
{
	gl_Position = vec4(position, 1.0);
	VertColor = color;
	TextCoord = textCoord;
}
```

**Step2** :然后需要设置OpenGL纹理参数；最后通过读取纹理图片，定义纹理图像格式等信息。纹理数据最终传递到了显卡中存储。

```C++
   // Section3 准备纹理对象
	// Step1 创建并绑定纹理对象
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	// Step2 设定wrap参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Step3 设定filter参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		GL_LINEAR_MIPMAP_LINEAR); // 为MipMap设定filter方法
	// Step4 加载纹理
	GLubyte *imageData = NULL;
	int picWidth, picHeight;
	imageData = SOIL_load_image("wood.png", 
		&picWidth, &picHeight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, picWidth, picHeight, 
		0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Step5 释放纹理图片资源
	SOIL_free_image_data(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);
```
**注意**  图片资源在创建完纹理后就可以释放了，使用SOIL_free_image_data完成。

**Step3** 着色器中使用纹理对象
在顶点着色器中我们传递了纹理坐标，有了纹理坐标，获取最终的纹素使用过在片元着色器中完成的。由于纹理对象通过使用uniform变量来像片元着色器传递，实际上这里传递的是对应纹理单元(texture unit)的索引号。纹理单元、纹理对象对应关系如下图所示：
![纹理单元](http://img.blog.csdn.net/20140924163347437)

着色器通过纹理单元的索引号索引纹理单元，每个纹理单元可以绑定多个纹理到不同的目标(1D,2D)。OpenGL可以支持的纹理单元数目，一般至少有16个，依次为GL_TEXTURE0  到GL_TEXTURE15，纹理单元最大支持数目可以通过查询GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS常量获取。这些常量值是按照顺序定义的，因此可以采用 GL_TEXTURE0 + i 的形式书写常量，其中整数i在[0, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)范围内。

**作为一个了解**，纹理对象不仅包含纹理数据，还包含采样参数，这些采样参数称之为采样状态(sampling state)。而采样对象(sampler object)就是只包含采样参数的对象，将它绑定到纹理单元时，它会覆盖纹理对象中的采样状态，从而重新配置采样方式。这里不再继续讨论采样对象的使用了。

要使用纹理必须在使用之前激活对应的纹理单元，默认状态下0号纹理单元是激活的，因此即使没有显式地激活也能工作。激活并使用纹理的代码如下：

```C++
  // 使用0号纹理单元
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(glGetUniformLocation(shader.programId, "tex"), 0); 
```
上述glUniform1i将0号纹理单元作为整数传递给片元着色器，片元着色器中使用uniform变量对应这个纹理采样器，使用变量类型为:

```
uniform sampler2D tex;
```
> **uniform变量与attribute变量**  uniform变量与顶点着色器中使用的属性变量(attribute variables)不同，
属性变量首先进入顶点着色器，如果要传递给片元着色器，需要在顶点着色器中定义输出变量输出到片元着色器。而uniform变量则类似于全局变量，在整个着色器程序中都可见。

完整的片元着色器代码为:
```C++
#version 330

in vec3 VertColor;
in vec2 TextCoord;

uniform sampler2D tex;

out vec4 color;


void main()
{
	color = texture(tex, TextCoord);
}
```
其中[texture](https://www.opengl.org/sdk/docs/man/html/texture.xhtml)函数根据纹理坐标，获取纹理对象中的纹素。
运行程序，效果如下图所示：

![二维纹理](http://img.blog.csdn.net/20161029101049012)

这里为绘制的矩形添加了纹理，可以从[我的github](https://github.com/wangdingqiao/noteForOpenGL/tree/master/textures)下载程序完整代码。

## 重构代码
将上面处理纹理部分的代码整理成一个函数，放在textureHelper类里，可以从我的github查看这个类的代码。使用textureHelper类加载纹理的代码为:
```C++
GLint textureId = TextureHelper::load2DTexture("wood.png");
```
在上面的顶点着色器中，我们也传递了顶点颜色属性，将顶点颜色和纹理混合，修改片元着色器中代码为：

```
color = texture(tex, TextCoord) * vec4(VertColor, 1.0f);
```

![二维纹理与颜色混合](http://img.blog.csdn.net/20161029101131263)


## 使用多个纹理单元
上面介绍了一个纹理单元支持多个纹理绑定到不同的目标，一个程序中也可以使用多个纹理单元加载多个2D纹理。使用多个纹理单元的代码如下:
```C++
shader.use();
// 使用0号纹理单元
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, textureId1);
glUniform1i(glGetUniformLocation(shader.programId, "tex1"), 0); 
// 使用1号纹理单元
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, textureId2);
glUniform1i(glGetUniformLocation(shader.programId, "tex2"), 1); 
```
在着色器中，对两个纹理的颜色进行混合：
```C++
   #version 330

in vec3 VertColor;
in vec2 TextCoord;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float mixValue;

out vec4 color;


void main()
{
	vec4 color1 = texture(tex1, TextCoord);
	vec4 color2 = texture(tex2, TextCoord);
	color = mix(color1, color2, mixValue);
}
```
其中[mix函数](https://www.opengl.org/sdk/docs/man/html/mix.xhtml)完成颜色插值，函数原型为：
> **API** genType mix( 	genType x,
  	genType y,
  	genType a);
  	
最终值得计算方法为：$x \times (1-a)+y \times a$。
mixValue通过程序传递，可以通过键盘上的A和S键，调整纹理混合值，改变混合效果。

运行效果如下:

![two texture unit](http://img.blog.csdn.net/20161029101209185)

画面中这只猫是倒立的，主要原因是加载图片时，图片的(0,0)位置一般在左上角，而OpenGL纹理坐标的(0,0)在左下角，这样y轴顺序相反。有的图片加载库提供了相应的选项用来翻转y轴，SOIL没有这个选项。我们可以修改顶点数据中的纹理坐标来达到目的，或者对于我们这里的简单情况使用如下代码实现y轴的翻转:
```C++
vec4 color2 = texture(tex2, 
	vec2(TextCoord.s, 1.0 - TextCoord.t));
```

修改后的运行效果如下所示：

![two texture unit mix](http://img.blog.csdn.net/20161029101248186)

上述程序完整的代码可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/textures)。

**说明** 限于时间关系，文中的示例图片部分来源于网络，均注明了出处，向原作者表示感谢。

## 参考资料
1. [Android Lesson Six: An Introduction to Texture Filtering](http://www.learnopengles.com/android-lesson-six-an-introduction-to-texture-filtering/)
2. www.learnopengl.com [Textures](http://www.learnopengl.com/#!Getting-started/Textures)
3. [Basic Texture Mapping ](http://ogldev.atspace.co.uk/www/tutorial16/tutorial16.html)
4. [Textures objects and parameters](https://open.gl/textures)
5. [Tutorial 5 : A Textured Cube](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/)


## 推荐阅读
1. 关于Texture filtering [Shawn Hargreaves Blog-Texture filtering](https://blogs.msdn.microsoft.com/shawnhar/2009/09/08/texture-filtering/)
2. 关于Mipmap的[Shawn Hargreaves Blog-Texture filtering: mipmaps](https://blogs.msdn.microsoft.com/shawnhar/2009/09/14/texture-filtering-mipmaps/)
