写在前面
    在绘制封闭类型的几何对象时，开启背面剔除功能能够提高渲染性能。本节简要介绍下背面剔除。


# 什么是背面剔除

当我们观察场景中对象时，一般只能以一定角度来观察，那么对象的某些面我们是看不到的，例如你观察一个立方体，最多只能同时看到3个面，有时只能看到1个面，而我们绘制时如果不采取剔除背面的措施，则要绘制6个面，其中包括一些，我们根本看不到的面。对于立方体这个面较少的几何对象，性能开销不明显，但是对于复杂的模型，开启背面剔除则能明显改善渲染性能。 背面剔除，就是早点丢弃对观察者来说是背面的片元的一种方法。

# 背面剔除的使用

上面提到，要早点丢弃对观察者来说是背面的片元，那么现在的问题是如何确定哪个面是背面的问题？ OpenGL中使用顶点绕序(winding order)来确定。所谓绕序就是当几何对象细分为三角形时，三角形顶点相对于中心的定义顺序，具体如下图所示（来自[opengl wiki](https://www.opengl.org/wiki/Face_Culling)）：

![顶点绕序](http://img.blog.csdn.net/20160821142020384)

左边的图中指定顶点的顺序是顺时针的，右边是逆时针的。

以三角面指向观察者的方向为大拇指指向，其余手指逆时针绕着大拇指，如果手指的绕向和三角面顶点绕向一致，则这个面为正面，否则为背面。

在OpenGL中，我们指定逆时针和顺时针的三角形顶点如下：

```cpp
   GLfloat vertices[] = {
    // Clockwise
    vertices[0], // vertex 1
    vertices[1], // vertex 2
    vertices[2], // vertex 3
    // Counter-clockwise
    vertices[0], // vertex 1
    vertices[2], // vertex 3
    vertices[1]  // vertex 2  
};
```

需要注意的是，三角形的正面还是背面这个是根据观察者的观察方向，而变动的。例如下面的图中(来自[www.learnopengl.com](http://www.learnopengl.com/#!Advanced-OpenGL/Face-culling))：

![正方向问题](http://img.blog.csdn.net/20160821142904990)

左侧的三角形顶点顺序为1->2->3,右侧的三角形顶点顺序为1->2->3。当观察者在右侧时，则右边的三角形方向为逆时针方向为正面，而左侧的三角形为顺时针则为背面；当观察者转到左侧时，左侧的三角形为逆时针绕序判定为正面，而右侧的三角形为顺时针绕序判定为背面。可以看出正面和背面是由三角形的顶点定义顺序和观察者的观察方向共同决定的，而且随着观察方向的改变，正面和背面将会跟着改变。

在OpenGL中开启背面剔除需要使用:

```cpp
   glEnable(GL_CULL_FACE);
```

同时OpenGL提供了函数[glCullFace](https://www.opengl.org/sdk/docs/man2/xhtml/glCullFace.xml)来供用户选择剔除哪个面：

> **API**  void glCullFace(GLenum  mode);
> mode参数为 GL_FRONT, GL_BACK,  GL_FRONT_AND_BACK 3个枚举类型。默认为GL_BACK.

以及[glFrontFace](https://www.opengl.org/sdk/docs/man2/xhtml/glFrontFace.xml)函数用来根据绕序指定哪个为正面:

> **API** void glFrontFace(GLenum  mode);
> mode参数为GL_CW ，GL_CCW。默认值是GL_CCW。

如果要剔除正面，代码:

```cpp
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
``` 

和下面的代码等价:

```cpp
   glCullFace(GL_FRONT);
```

首先我们来对比下使用背面剔除和未使用背面剔除时，穿过立方体内部，我们看到的景象。下图是没有开启背面剔除时：

![未开启背面剔除](http://img.blog.csdn.net/20160821144644495)

可以看到，穿过内部时，看到了立方体后面的部分。当开启了背面剔除时，穿过内部时，看不到立方体的背面：

![开启了背面剔除](http://img.blog.csdn.net/20160821144557308)

当我们剔除正面时，观察者的位置，决定了哪是正面，当观察者处于立方体正面时，剔除正面看到效果：

![剔除1个正面](http://img.blog.csdn.net/20160821144920059)

当观察者移动到可以看到立方体三个面时，判定正面，顶面和右侧面为正面，则剔除正面后的效果为：

![剔除三个正面](http://img.blog.csdn.net/20160821145034003)

# 最后的说明
在使用背面剔除时需要注意，在指定物体表面的顶点时，顶点的顺序需要定义为正向的逆时针顺序。启用背面剔除一般而言总能改善渲染性能，但是具体是否开启背面剔除还是要根据应用场景而定。当渲染某些非封闭类型几何对象，例如一颗草的模型时，则不需要开启背面剔除。