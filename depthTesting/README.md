写在前面
  [上一节](http://blog.csdn.net/wangdingqiaoit/article/details/52014321)我们使用AssImp加载了3d模型，效果已经令人激动了。但是绘制效率和场景真实感还存在不足，接下来我们还是要保持耐心，继续学习一些高级主题，等学完后面的高级主题，我们再次来改进我们加载模型的过程。本节将会学习深度测试，文中示例程序源代码均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/depthTesting)。
  
> 本节内容整理自
> 1.[www.learnopengl.com Depth testing](http://www.learnopengl.com/#!Advanced-OpenGL/Depth-testing)
> 2.[depth buffer faq](https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm)
> 3.[Z buffer 和 W buffer 簡介](https://www.csie.ntu.edu.tw/~r89004/hive/hsr/page_1.html)

通过本节可以了解到

- 为什么需要深度缓冲区？
- OpenGL中怎么使用深度缓冲区 ？
- 可视化深度值
- 深度值的精度问题-ZFighting


# 问题背景
在绘制3D场景的时候，我们需要决定哪些部分对观察者是可见的，或者说哪些部分对观察者不可见，对于不可见的部分，我们应该及早的丢弃，例如在一个不透明的墙壁后的物体就不应该渲染。这种问题称之为[隐藏面消除](https://en.wikipedia.org/wiki/Hidden_surface_determination)（Hidden surface elimination）,或者称之为找出可见面(Visible surface detemination)。

解决这一问题比较简单的做法是画家算法([painter's algorithm](https://en.wikipedia.org/wiki/Painter%27s_algorithm))。画家算法的基本思路是，先绘制场景中离观察者较远的物体，再绘制较近的物体。例如绘制下面图中的物体(来自[Z buffer 和 W buffer 簡介](https://www.csie.ntu.edu.tw/~r89004/hive/hsr/page_1.html))，先绘制红色部分，再绘制黄色，最后绘制灰色部分，即可解决隐藏面消除问题。

![画家算法举例](http://img.blog.csdn.net/20160807203914290)

使用画家算法时，只要将场景中物体按照离观察者的距离远近排序，由远及近的绘制即可。画家算法很简单，但另一方面也存在缺陷，例如下面的图中，三个三角形互相重叠的情况，画家算法将无法处理：

![画家算法失效](http://img.blog.csdn.net/20160807203943775)

解决隐藏面消除问题的算法有很多，具体可以参考[Visible Surface Detection](http://www.tutorialspoint.com/computer_graphics/visible_surface_detection.htm)。结合OpenGL，我们使用的是Z-buffer方法，也叫深度缓冲区Depth-buffer。

深度缓冲区(Detph buffer)同颜色缓冲区(color buffer)是对应的，颜色缓冲区存储的像素的颜色信息，而深度缓冲区存储像素的深度信息。在决定是否绘制一个物体的表面时，首先将表面对应像素的深度值与当前深度缓冲区中的值进行比较，如果大于等于深度缓冲区中值，则丢弃这部分;否则利用这个像素对应的深度值和颜色值，分别更新深度缓冲区和颜色缓冲区。这一过程称之为深度测试(Depth Testing)。在OpenGL中执行深度测试时，我们可以根据需要指定深度值的比较函数，后面会详细介绍具体使用。

# OpenGL中使用深度测试

深度缓冲区一般由窗口管理系统，例如GLFW来创建，深度值一般由16位，24位或者32位值表示，通常是24位。位数越高的话，深度的精确度越好。前面我们已经见过了如何在OpenGL中使用深度测试，这里复习下过程。首先我们需要开启深度测试，默认是关闭的：

```cpp
   glEnable(GL_DEPTH_TEST);
```

另外还需要在绘制场景前，清除颜色缓冲区时，清除深度缓冲区：

```cpp
   glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```
清除深度缓冲区的默认值是1.0，表示最大的深度值，深度值的范围在[0,1]之间，值越小表示越靠近观察者，值越大表示远离观察者。
上面提到了在进行深度测试时，当前深度值和深度缓冲区中的深度值，进行比较的函数，可以由用户通过glDepthFunc指定，这个函数包括一个参数，具体的参数如下表所示：

| 函数       | 说明        |
|----------|-----------|
|GL_ALWAYS 	|总是通过测试|
|GL_NEVER 	|总是不通过测试|
|GL_LESS 	|在当前深度值 < 存储的深度值时通过|
|GL_EQUAL 	|在当前深度值 = 存储的深度值时通过|
|GL_LEQUAL 	|在当前深度值 <= 存储的深度值时通过|
|GL_GREATER 	|在当前深度值 > 存储的深度值时通过|
|GL_NOTEQUAL 	|在当前深度值 不等于 存储的深度值时通过|
|GL_GEQUAL 	|在当前深度值 >= 存储的深度值时通过|

例如我们可以使用GL_AWALYS参数，这与默认不开启深度测试效果是一样的：

```cpp
  glDepthFunc(GL_ALWAYS);
```
下面我们绘制两个立方体和一个平面，通过对比开启和关闭深度测试来理解深度测试。 
当关闭深度测试时，我们得到的效果却是这样的：
![没有启用深度测试](http://img.blog.csdn.net/20160807204046568)
这里先绘制立方体，然后绘制平面，如果关闭深度测试，OpenGL只根据绘制的先后顺序决定显示结果。那么后绘制的平面遮挡了一部分先绘制的本应该显示出来的立方体，这种效果是不符合实际的。

我们开启深度测试后绘制场景，得到正常的效果如下：
![这里写图片描述](http://img.blog.csdn.net/20160807204157356)

使用深度测试，最常见的错误时没有使用`glEnable(GL_DEPTH_TEST);`
开启深度测试，或者没有使用`glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);`清除深度缓冲区。

与深度缓冲区相关的另一个函数是[glDepthMask](https://www.opengl.org/sdk/docs/man2/xhtml/glDepthMask.xml),它的参数是布尔类型，GL_FALSE将关闭缓冲区写入，默认是GL_TRUE，开启了深度缓冲区写入。



#可视化深度值
在可视化深度值之前，首先我们要明白，这里的深度值，实际上是屏幕坐标系下的$z_{win}$坐标，屏幕坐标系下的(x,y)坐标分别表示屏幕坐标系下以左下角(0,0)为起始点的坐标。$z_{win}$我们如何获取呢？ 可以通过着色器的输入变量gl_FragCoord.z来获取，这个gl_FragCoord的z坐标表示的就是深度值。

我们在着色器中以这个深度值为颜色输出：

```cpp
  // 原样输出
float asDepth()
{
  return gl_FragCoord.z;
}
void main()
{
	float depth = asDepth();
	color = vec4(vec3(depth), 1.0f);
}
```
输出后的效果如下图所示：
![默认深度值可视化](http://img.blog.csdn.net/20160807204826801)

可以看到图中，只有离观察者较近的部分有些黑色，其余的都是白色。这是因为深度值$z_{win}$和$z_{eye}$是成非线性关系的，在离观察者近的地方，精确度较高，$z_{win}$值都保持在较小范围，成黑色。但是一旦超出一定距离，精确度变小，$z_{win}$值都挤在1.0附近，因此成白色。当我们向后移动，拉远场景与观察者的距离后，$z_{win}$值都落在1.0附近，整个场景都变成白色，如下图所示：
![当拉远场景与观察者距离后深度值都变为1.0](http://img.blog.csdn.net/20160807205207978)

作为深度值的可视化，我们能不能使用线性的关系来表达$z_{win}$和$z_{eye}$ ? 这里我们做一个尝试，从$z_{win}反向推导出$z_{eye}$。在[投影矩阵和视口变换矩阵](http://blog.csdn.net/wangdingqiaoit/article/details/51589825)一节，我们计算出了相机坐标系下坐标和规范化设备坐标系下坐标之间的关系如下：

$$\begin{align} z_{ndc} &=\frac{-\frac{f+n}{f-n}z_{eye}-\frac{2fn}{f-n}}{-z_{eye}} \\
&=\frac{f+n}{f-n}+\frac{2fn}{(f-n)z_{eye}}
\end{align}\tag{1}$$

在OpenGL中从规范化设备坐标系转换到屏幕坐标系使用函数主要是：
[glViewport(GLint  $s_{x}$ , GLint $s_{y}$ , GLsizei $w_{s}$ , GLsizei  $h_{s}$)](https://www.opengl.org/sdk/docs/man/html/glViewport.xhtml);
[glDepthRangef(GLclampf $n_{s}$ , GLclampf $f_{s}$ )](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glDepthRangef.xml);

继而可以得到规范化设备坐标系和屏幕设备坐标系之间的关系如下：

$z_{win}=\frac{f_{s}-n_{s}}{2}z_{ndc}+\frac{f_{s}+n_{s}}{2}\tag{2}$

默认情况下glDepthRange函数的n=0,f=1,因此从（2）式可以得到：
$z_{win}=\frac{1}{2}z_{ndc}+\frac{1}{2}\tag{3}$
$z_{ndc}=2z_{win}-1\tag{4}$

从式子（1）我们可以得到：
$z_{eye}=\frac{2fn}{z_{ndc}(f-n)-(f+n)}\tag{5}$

上面的式子(5)如果用来作为深度值，由于结果是负数，会被截断到0.0，结果都是黑色，因此我们对分母进行反转，写为式子(6)作为深度值。

$z_{eye}=\frac{2fn}{(f+n)-z_{ndc}(f-n)}\tag{6}$

对式子(6)的深度值进行归一化，保持在[0,1]范围内，则在着色器中实现为：

```cpp

// 线性输出结果
float near = 1.0f; 
float far  = 100.0f; 
float LinearizeDepth() 
{
    // 计算ndc坐标 这里默认glDepthRange(0,1)
    float Zndc = gl_FragCoord.z * 2.0 - 1.0; 
    // 这里分母进行了反转
    float Zeye = (2.0 * near * far) / (far + near - Zndc * (far - near)); 
	return (Zeye - near)/ ( far - near);
}
void main()
{
	float depth = LinearizeDepth();
	color = vec4(vec3(depth), 1.0f);
}
```

使用$z_{win}$和$z_{eye}$线性关系得到深度值，绘制的效果如下图所示：
![深度值线性可视化](http://img.blog.csdn.net/20160807204928755)

很多网络教程都近似表达$z_{win}$和$z_{eye}$的非线性关系，用来可视化，我们可以从(4)(6)得到非线性关系：
$z_{win}=\frac{\frac{1}{n}-\frac{1}{z_{eye}}}{{\frac{1}{n}}-{\frac{1}{f}}} \tag{7}$
在着色器中实现为：

```cpp
   // 非线性输出
float nonLinearDepth()
{
	float Zndc = gl_FragCoord.z * 2.0 - 1.0; 
    float Zeye = (2.0 * near * far) / (far + near - Zndc * (far - near)); 
	return (1.0 / near - 1.0 / Zeye) / (1.0 / near - 1.0 / far);
}
void main()
{
	float depth = nonLinearDepth();
	color = vec4(vec3(depth), 1.0f);
}
```
这个非线性关系输出，和利用gl_FragCoord.z作为深度值输出效果是差不多的。

# 深度的精确度问题-ZFighting
实际使用时不使用$z_{win}$和$z_{eye}$的线性关系，因为在场景中，近处的物体，我们想让它看的清楚，自然 要求精度高；但是远处的物体，我们不需要很清晰的看到细节，因此精确度不必和近处的物体一样。使用公式(7)绘制的$z_{win}$和$z_{eye}$关系图如下所示(来自：[www.learnopengl.com Depth testing](http://www.learnopengl.com/#!Advanced-OpenGL/Depth-testing))：

![非线性深度值关系图](http://img.blog.csdn.net/20160807205443654)

我们看到，$z_{eye}$在[1.0,2.0]范围内时$z_{win}$保持在0.5的范围内，精确度高。而当$z_{eye}$超过10.0后，$z_{win}$的值就在0.9以后了，也就是说$z_{win}$在[10.0,50.0]范围内的深度值将挤在[0.9,1.0]这么一个小的范围内，精确度很低。

实际上深度值是通过下式计算的(来自:[depth buffer faq](https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm))：
$z_{win}=S*(\frac{fn}{(f-n)z_{eye}}+\frac{1}{2}\frac{f+n}{f-n}+\frac{1}{2})\tag{8}$
其中，$S=2^{d}-1$，d表示深度缓冲区的位数(例如16,24,32)。这个式子的右边括号部分是由(1)(4)得到，同时放大S倍数后得到最终的深度值(可以参看[depth buffer faq](https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm))。

找到两个特殊点，$z_{win} =1$和$z_{win} = S-1$，得到：
$z_{win} = 1 => z_{eye} = f * n / ((1/s) * (f-n) - f)$
$z_{win} = S-1 => z_{eye} = f * n / (((s-1)/s) * (f-n) - f)$

取n = 0.01, f = 1000 and s = 65535，那么有：
$z_{win} = 1 => z_{eye}= -0.01000015$
$z_{win}  = S-1 => z_{eye}= -395.90054$

注意OpenGL中相机坐标系的+Z轴指向观察者，因此上面的坐标是负数。从上面的值我们可以看到，当$z_{eye}$在[-395,-1000]范围内时，深度值将全部挤在65534或者65535这两个值上，也就是说几乎60%的$z_{eye}$只能分配1到2个深度值，可见当$z_{eye}$超过一定范围后，精度值是相当低的。(这个例子原本解释来自[depth buffer faq](https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm))。

当深度值精确度很低时，容易引起ZFighting现象，表现为两个物体靠的很近时确定谁在前，谁在后时出现了歧义。例如上面绘制的平面和立方体，在y=-0.5的位置二者贴的很近，如果进入立方体内部观察，则出现了ZFighting现象，立方体的底面纹理和平面的纹理出现了交错现象，如下图所示：

![ZFighting1](http://img.blog.csdn.net/20160807205832528)

![ZFighting2](http://img.blog.csdn.net/20160807205950035)

（如果你要亲自观察这个现象，只需要在本节代码中，将相机位置放在立方体内部，稍微调整鼠标观察角度就可以了）。

# 预防ZFighting的方法

1.不要将两个物体靠的太近，避免渲染时三角形叠在一起。这种方式要求对场景中物体插入一个少量的偏移，那么就可能避免ZFighting现象。例如上面的立方体和平面问题中，将平面下移0.001f就可以解决这个问题。当然手动去插入这个小的偏移是要付出代价的。
2.尽可能将近裁剪面设置得离观察者远一些。上面我们看到，在近裁剪平面附近，深度的精确度是很高的，因此尽可能让近裁剪面远一些的话，会使整个裁剪范围内的精确度变高一些。但是这种方式会使离观察者较近的物体被裁减掉，因此需要调试好裁剪面参数。
3.使用更高位数的深度缓冲区，通常使用的深度缓冲区是24位的，现在有一些硬件使用使用32位的缓冲区，使精确度得到提高。

当然还有其他方法，这里不再展开了。

# 最后的说明
本节了解了深度测试的问题背景，OpenGL中的使用方法。通过可视化深度值和给出深度的计算过程，让我们了解深度的精确度问题。还有一些问题没有在本节探讨，包括gl_FragCoord,gl_FragDepth的含义和计算方法，等待后面再继续学习。另外关于fragment,pixel的区别还需要做进一步了解，本文关于这部分的表述还有待改善。


# 参考资料
1.[www.learnopengl.com Depth testing](http://www.learnopengl.com/#!Advanced-OpenGL/Depth-testing)
2.[深度值计算 Real depth in OpenGL / GLSL](http://web.archive.org/web/20130416194336/http://olivers.posterous.com/linear-depth-in-glsl-for-real)
3.[提供了深度值在线计算程序](https://www.sjbaker.org/steve/omniv/love_your_z_buffer.html)
4.[opengl wiki Depth_Buffer_Precision](https://www.opengl.org/wiki/Depth_Buffer_Precision)
5.[Z-buffering](https://en.wikipedia.org/wiki/Z-buffering)
6.上面提到的线性和非线性的计算方法 [SO讨论](http://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer)