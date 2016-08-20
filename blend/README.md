写在前面
  上一节学习了[使用模板缓冲来制作特殊效果](http://blog.csdn.net/wangdingqiaoit/article/details/52143197)，本节将继续学习一个高级主题-混色(Blending)。通过使用混色，我们可以制作透明、半透明效果。本节示例代码均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/blend)。

> 本节内容整理自[www.learnopengl.com blending](http://www.learnopengl.com/#!Advanced-OpenGL/Blending).

# 混色的概念

所谓混色，就是将当前要绘制的物体的颜色和颜色缓冲区中已经绘制了的物体的颜色进行混合，最终决定了当前物体的颜色。例如下面的图中，狙击枪的瞄准器本身是带有蓝色的，将它和后面的任务混合在一起，形成了我们看到的最终效果，这个效果里既有瞄准器的蓝色成分，也有后面人物的像素，主要是后面人物的像素。

![颜色混合](http://img.blog.csdn.net/20160820220441288)

实际上我们通过玻璃看到外面的景象就是一种混色，有的玻璃完全透明则主要显示外面的景象，而另一些玻璃不是完全透明则成像中包含一部分玻璃的颜色。在OpenGL中使用混色，可以实现很多效果，其中比较常见的就是透明效果。下面具体实现完全透明和半透明效果。

# 完全透明效果

完全透明表现的是，当前物体例如透明玻璃，将后面的像素完全展示出来，而当前物体则不必显示。实现完全透明效果，我们通过对物体的透明度进行判断，当小于一定阈值，例如0.1时，我们则丢弃该片元，使其后面的片元得到显示。
首先我们加载一个草的模型，对于草这种模型，它要么完全透明，可以透过它看到后面的物体，要么不透明展示为草的细节。绘制草这种模型时，我们通过往矩形块上添加草的纹理来实现。加载了草的模型，使用[深度测试一节的立方体](http://blog.csdn.net/wangdingqiaoit/article/details/52206602)，绘制出来的效果如下：

![没有使用透明](http://img.blog.csdn.net/20160820221254979)

这里我们看到，草模型中透明部分和不透明部分没有得到区分，因而挡住了后面的立方体和草模型。在RGBA表达的颜色重，alpha成分一直以来，我们都是设置为1.0，实际上这个分量表达的就是透明度。1.0表示为完全不透明，0.0则表示完全透明。我们可以根据加载的草模型的alpha值判断是否应该丢弃片元来实现透明效果。

加载RGBA模型，和之前一直实现的加载RGB模型，有少许不同，我们要注意两点:

- 使用SOIL库的时候参数要从SOIL_LOAD_RGB改为SOIL_LOAD_RGBA

- glTexImage2D中图片格式和内部表示要从GL_RGB改为GL_RGBA.

- glTexParameteri纹理的wrap方式需要从GL_REPEAT改为GL_CLAMP_TO_EDGE，这个主要是为了防止由于使用GL_REPEAT时纹理边缘部分插值导致出现我们不需要的半透明的效果

加载纹理的函数声明为:
```cpp
static  GLuint load2DTexture(const char* filename, GLint internalFormat = GL_RGB,
		GLenum picFormat = GL_RGB, int loadChannels = SOIL_LOAD_RGB, GLboolean alpha=false);
```
完整的实现可以参考[texture.h](https://github.com/wangdingqiao/noteForOpenGL/blob/master/blend/transparent/texture.h)。

在代码中加载纹理变更为:


```cpp
GLuint transparentTextId =TextureHelper::load2DTexture(
"grass.png", GL_RGBA, GL_RGBA, SOIL_LOAD_RGBA, true);
```

在片元着色器中，根据alpha值是否小于设定的阈值，我们决定是否丢弃片元:

```cpp
#version 330 core
in vec2 TextCoord;
uniform sampler2D text;
out vec4 color;
void main()
{
	vec4 textColor = texture(text, TextCoord);
	if(textColor.a < 0.1)  // < 0.1则丢弃片元 
		discard;
	color = textColor;
}
```
这种方法实现的透明效果如下图所示：

![透明效果](http://img.blog.csdn.net/20160820223004561)

使用alpha值决定是否丢弃片元，我们实现的透明效果是要么完全透明(alpha <0.1)，要么不透明(alpha >= 0.1)。实际应用中还需要使用半透明效果。

# OpenGL中混色计算

混色后可以通过当前物体看到其后的物体，这里当前物体的最终颜色是由当前物体的颜色(**源的颜色 source color**)和颜色缓冲区中的颜色(**目的颜色 destination color**)混色决定的，也就是进行相应的混合计算得到的。

要开启混色功能需要使用:

```cpp
   glEnable(GL_BLEND);
```
混色是计算出来的，主体的公式是这样的:

$Result = source * sfactor+ destination * dfactor \tag{1}$

公式1中source和destination表示的分别是源和目的颜色，sFactor 和dFactor分别表示源和目的颜色的计算系数。
用户可以灵活的控制公式1的sFactor 和dFactor ，上式计算是逐个颜色分量RGBA计算的。

OpenGL提供了函数[glBlendFunc](https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml)用来设置上面的sfactor和dfactor,函数原型为:

> **API** void glBlendFunc( 	GLenum sfactor,
  	GLenum dfactor);
  	sfactor和dfactor用来指定源和目的颜色计算的系数，使用的是GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR等枚举值。

例如，一个红色和绿色方块进行混色，效果如下图所示：

![颜色混合](http://img.blog.csdn.net/20160820230542440)

这里绿色(0.0,1.0,0.0,0.6)作为源，红色(1.0,0.0,0.0,1.0)作为目的颜色进行混合。我们设置参数为：
```cpp
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
```
则进行计算的过程为：

$result = (0.0,1.0,0.0,0.6)*(0.6,0.6,0.6,0.6) + (1.0,0.0,0.0,1.0)*(0.4,0.4,0.4,0.4)=(0.4,0.6,0.0,0.76)$

除了glBlendFunc外，还可以使用使用[glBlendFuncSeparate](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glBlendFuncSeparate.xml)单独指定RGB，Alpha的计算系数。

> **API** void glBlendFuncSeparate(GLenum srcRGB,  GLenum dstRGB,  GLenum srcAlpha,  GLenum dstAlpha);
这里的参数同样是GL_ZERO,GL_ONE,GL_SRC_COLOR等枚举值。

另外，还可以通过
[glBlendEquation(GLenum mode);](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glBlendEquation.xml)和[glBlendEquationSeparate](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glBlendEquationSeparate.xml)来指定源和目的颜色的计算方式，默认是GL_FUNC_ADD，就是公式1所示的情况。例如GL_FUNC_SUBTRACT则对应公式2：

$Result = source * sfactor - destination * dfactor \tag{2}$

一般我们使用的组合为:

```cpp
  glBlendEquation(GL_FUNC_ADD); // 默认，无需显式设置
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  
```

# 绘制半透明效果

上面介绍了OpenGL中混色的计算，下面实现一个半透明的效果。
通过加载一个半透明的窗户到场景，使得透过窗户可以看到后面的场景。我们的着色器恢复到：

```cpp
#version 330 core
in vec2 TextCoord;
uniform sampler2D text;
out vec4 color;

void main()
{
    color = texture(text, TextCoord);
}
```
在场景中使用GL_RGBA等包含alpha的参数加载窗户模型后，绘制窗户时使用代码:

```cpp
 for (std::vector<glm::vec3>::const_iterator it = windowObjs.begin();
	windowObjs.end() != it; ++it)
{
	model = glm::mat4();
	model = glm::translate(model, *it);
	glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
		1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
```
得到效果如下图所示：

![没有进行transparent sort](http://img.blog.csdn.net/20160820234034268)

上面的图中，仔细看则会发现视觉bug,前面的窗户看不到后面的窗户。这主要是因为深度测试，并不关心alpha值，因此前面的窗户由于里观察者更近，挡住了后面的窗户，因此后面的窗户没有显示出来。
对于这一问题，需要考虑[排序问题 Transparency Sorting](https://www.opengl.org/wiki/Transparency_Sorting)。
绘制包含不透明和透明场景的顺序为：

1.首先绘制不透明物体
2.对透明物体进行排序
3.按照排序后的顺序，绘制透明物体。

我们这里的解决方法是对窗户进行由远及近的绘制，那么在绘制近一些的窗户时，执行混色，混合当前颜色buffer中颜色(场景中处于后面的窗户的颜色)和当前要绘制的窗户颜色，则能产生正常的结果。

这里使用的排序规则是，窗户到观察者的距离，借助c++ std::map默认对键值进行排序的功能排序，然后使用逆向迭代器迭代绘制即可，具体实现为：

```cpp
// 绘制透明物体
// 根据到观察者距离远近排序 使用map的键的默认排序功能(键为整数时从小到大)
std::map<GLfloat, glm::vec3> distanceMap;
for (std::vector<glm::vec3>::const_iterator it = windowObjs.begin();
windowObjs.end() != it; ++it)
{
	float distance = glm::distance(camera.position, *it);
	distanceMap[distance] = *it;
}
transparentShader.use();
glBindVertexArray(transparentVAOId);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, transparentTextId);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// 根据transparency sort 结果进行绘制
for (std::map<GLfloat, glm::vec3>::reverse_iterator it = distanceMap.rbegin();
distanceMap.rend() != it; ++it)
{
	model = glm::mat4();
	model = glm::translate(model, it->second);
	glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
	1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
glDisable(GL_BLEND);
```

最终的半透明效果为：

![半透明效果](http://img.blog.csdn.net/20160820234633055)

# 最后的说明

本节介绍了使用混色功能绘制透明和半透明效果。注意在加载纹理时，如果没有将wrap方式从GL_REPEAT改为GL_CLAMP_TO_EDGE将会得到错误的效果，如下图所示：

![边缘错误](http://img.blog.csdn.net/20160820235306236)

同时本节在绘制半透明的窗户时，解决前后窗户的视觉bug采用的排序规则是使用窗户离观察者的距离，这一方法并不适合所有的情形。实际在进行Blend时解决这一个问题的方式是复杂的，感兴趣地可以参考[Transparency Sorting](https://www.opengl.org/wiki/Transparency_Sorting)。同时混色也可以在指定的buffer上执行，感兴趣地可以参考[OpenGL wiki Blending](https://www.opengl.org/wiki/Blending).
