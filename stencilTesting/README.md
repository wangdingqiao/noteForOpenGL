写在前面
上一节介绍了[深度测试](http://blog.csdn.net/wangdingqiaoit/article/details/52206602)，本节继续学习一个高级主题-模板测试(stencil testing)。模板缓冲同之前介绍的颜色缓冲、深度缓冲类似，通过它我们可以实现很多的特效，例如轮廓、镜面效果，阴影效果等。本节示例程序均可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/stencilTesting)。

通过本节可以了解到

- 模板缓冲的作用
- 模板缓冲的使用方法-简单的矩形模板
- 模板缓冲实现的outline和refleciton效果

# 模板缓冲的作用
上一节介绍的深度缓冲用于进行深度测试，决定场景中物体的表面是否可见，解决隐藏面消除的问题，简而言之，就是通过深度测试，OpenGL选择性渲染片元。模板测试的是另外一种可以以一定标准丢弃片元的方法，这个标准就是借助模板缓冲和我们指定的测试函数而运作的。实际上渲染管线里面包括几种测试，如下图所示（来自[Improving Shadows and Reflections via the Stencil Buffer](http://artis.imag.fr/Recherche/RealTimeShadows/pdf/stencil.pdf)）：
![渲染管线](http://img.blog.csdn.net/20160814200325521)

我们这里展开上面图中所有内容，但是我们看到模板测试是在深度测试之前进行的，可以作为一种丢弃片元的辅助方法。

# 模板缓冲的使用
模板缓冲一般为8位的，存贮整数，最大值为255。在使用的过程中步骤一般时，开启模板缓冲，绘制一个物体作为我们的模板，这个过程实际上就是写入模板缓冲的过程；接着我们利用模板缓冲中的值决定是丢弃还是保留后续绘图中的片元。下面我们建立一个举行模板，通过矩形模板选择性地将上一节绘制的场景显示出来，这个过程示意如下图所示（来自[Stencil testing](http://www.learnopengl.com/#!Advanced-OpenGL/Stencil-testing)）：
![举行模板](http://img.blog.csdn.net/20160814200912952)

如图中所示，模板缓冲中为1的地方我们选择保留图形，而其他部分则丢弃，形成最终的效果。

使用模板缓冲需要三个要素：

- 正确的时间开启和关闭深度缓冲  
- 模板测试函数
- 模板测试函数失败或者成功后的执行的动作

在OpenGL中开启模板缓冲的方法如下：

```cpp
   glEnable(GL_STENCIL_TEST);
```
同时和深度缓冲一样，需要清除，默认清除时写入0，可以通过glClearStencil设置清除的指定值。

```cpp
   glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
```
一般地绘制模板以及利用模板选择性地绘制物体时则开启模板缓冲，绘制其他物体时关闭模板缓冲。使用模板缓存的步骤一般如下：

1. 开启模板测试
2. 绘制模板，写入模板缓冲(不写入color buffer和depth buffer)
3. 关闭模板缓冲写入
4. 利用模板缓冲中的值，绘制后续场景

# 与模板测试相关的函数

[glStencilMask](https://www.opengl.org/sdk/docs/man/html/glStencilMask.xhtml) 函数用于控制模板缓冲区的写入，使用位掩码的方式决定是否可以写入模板缓冲区，使用得较多的是0x00表示禁止写入，0xFF表示允许任何写入。

[glStencilFunc ](https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glStencilFunc.xml)，用于指定模板测试的函数，这里指定是什么情况下通过模板测试。
> **API** void glStencilFunc(GLenum func,  GLint ref,  GLuint mask);
func同深度测试一样，指定函数名GL_NEVER,GL_LESS,GL_LEQUAL等函数。
ref是和当前模板缓冲中的值stencil进行比较的指定值，这个比较方式使用了第三个参数mask，例如GL_LESS通过，当且仅当
 满足: ( ref & mask ) < ( stencil & mask ).GL_GEQUAL通过，当且仅当( ref & mask ) >= ( stencil & mask )。

一般地，我们将上述函数的mask置为0xFF，用于比较，则比较时计算方式比较直观。例如:

```cpp
glStencilFunc(GL_EQUAL, 1, 0xFF)
```
表示当前模板缓冲区中值为1的部分通过模板测试，这部分片元将被保留，其余地则被丢弃。

[glStencilOp ](https://www.opengl.org/sdk/docs/man2/xhtml/glStencilOp.xml)用于指定测试通过或者失败时执行的动作，例如保留缓冲区中值，或者使用ref值替代等操作。

> **API** void glStencilOp(GLenum  sfail,  GLenum  dpfail,  GLenum  dppass);
sfail表示深度测试失败，dpfail表示模板测试通过但是深度测试失败，dppass表示深度测试成功。GLenum部分填写的是对应条件下执行的动作，例如GL_KEEP表示保留缓冲区中值，GL_REPLACE表示使用glStencilFunc设置的ref值替换。更完整的参数列表可以参考[glStencilOp](https://www.opengl.org/sdk/docs/man2/xhtml/glStencilOp.xml)。

这三个测试的关系如下图所示(来自[cnblogs 迈克老狼2012 depth/stencil buffer的作用](http://www.cnblogs.com/mikewolf2002/archive/2012/05/15/2500867.html))：
![三个测试的关系](http://img.blog.csdn.net/20160814204042400)


# 绘制矩形模板
首先实现上面给出的矩形模板，通过矩形模板我们将场景中不被矩形覆盖的部分丢弃，最终只显示一个矩形遮盖的区域。
绘制流程用代码简要地表示如下：

```cpp
// 清除颜色缓冲区 深度缓冲区 模板缓冲区
glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
// section1 绘制模板
glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
glDepthMask(GL_FALSE);
glStencilMask(0xFF);
glStencilFunc(GL_ALWAYS, 1, 0xFF);
// 在模板测试和深度测试都通过时更新模板缓冲区
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 

stencilShader.use();
glBindVertexArray(stencilVAOId);
glDrawArrays(GL_TRIANGLES, 0, 6);
// section 2绘制实际场景
glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
glDepthMask(GL_TRUE);
glStencilMask(0x00); // 禁止写入stencil
glStencilFunc(GL_EQUAL, 1, 0xFF);
glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
shader.use();
// 绘制第一个立方体
// 绘制第二个立方体
// 绘制平面
```
需要注意的是，绘制模板时使用代码：

```cpp
 glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
glDisable(GL_DEPTH_TEST);
```
禁止写入颜色和深度缓冲区，因为矩形模板最终是不显示在屏幕上的。后面我们可以看到，有些时候模板也需要显示，注意在合适的时候调整这部分代码。

最终的效果如下图所示：
![举行模板](http://img.blog.csdn.net/20160814204941513)
                    

# outline 轮廓效果
这部分参考自[www.learnopengl.com Stencil testing](http://www.learnopengl.com/#!Advanced-OpenGL/Stencil-testing)。

outline就是轮廓的效果，在游戏场景中，例如玩家选取了附件的物体时，通过轮廓线条来表示选取了哪些物体，十分有用。实现的思路是：

- 现正常比例绘制物体，同时绘制的部分作为模板

- 适当放大比例，在刚绘制的物体上绘制一个轮廓，使用模板缓冲实现。

实现的轮廓效果如下：

![轮廓效果](http://img.blog.csdn.net/20160814205526328)

这里第一次绘制了原始的立方体，通过原始立方体填充了模板缓冲区为1，第二次绘制放大一点的立方体，对比缓冲区中值等于1则丢弃，因此只显示处理轮廓部分。缓冲区比较函数实现为：

```cpp
 //	使用模板缓冲区 绘制立方体边缘
outlineShader.use();
glStencilMask(0x00); // 禁止写入模板缓冲区
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
glDisable(GL_DEPTH_TEST); // 这里关闭深度测试 是为了让轮廓不因为处在前面的平面而被消去
const GLfloat scale = 1.1f;
glBindVertexArray(cubeVAOId);
// 绘制第一个立方体轮廓
// 绘制第二个立方体轮廓
```

需要注意的是绘制轮廓线条时，需要暂时关闭深度测试，否则眼前的平面会把底部的轮廓线条遮挡掉，如果忘记了关闭深度测试，那么错误的效果如下：

![轮廓线错误](http://img.blog.csdn.net/20160814212721639)

需要注意地是正确地开启和允许模板缓冲区的读写，例如如果在第一次主循环结束之前忘记使用代码:

```cpp
   glEnable(GL_STENCIL_TEST);
   glStencilMask(0xFF);
```
开启和允许模板缓冲写操作，那么得到的错误效果如下：

![错误的效果](http://img.blog.csdn.net/20160814205800079)


# planar reflections 镜面效果
镜面效果表示的是物体的原始和反射后的图像，形成的一种镜面效果。实现思路为:

- 绘制镜面模板
- 利用模板，使用镜像变换(reflection transformation)，绘制反射后物体
- 使用混色(blend)绘制镜面
- 绘制原始物体

实现效果如下:

![镜面效果](http://img.blog.csdn.net/20160814210124955)

实现绘制反射的物体，主要通过向缩放操作传递负数来实现镜像变换，如果对镜像变换不熟悉，可以回过头去参考[模型变换(model transformation) ](http://blog.csdn.net/wangdingqiaoit/article/details/51531002)。需要注意地是，在本节代码中镜面位置在y=-0.5,那么镜像变换时，需要注意镜像变换的中心问题。这里关于y=-0.5变换，而不是y=0即x轴变换，因此首先将物体从变换中心(0.0,-0.5)移动到原点(0,0)，缩放后将物体再次移动到(0.0,-0.5)，这一过程表示为：

```cpp
  // 绘制第一个立方体反射
glm::mat4 firstReflectModel;
firstReflectModel = glm::translate(firstReflectModel, 
glm::vec3(-1.0f, 0.0f, -1.0f)); // 移动到与原始物体对应位置
// T*S*T形式地缩放
firstReflectModel =glm::translate(firstReflectModel, 
glm::vec3(0.0f, -0.5f, 0.0f));
firstReflectModel = glm::scale(firstReflectModel, glm::vec3(1.0f, -1.0f, 1.0f));
firstReflectModel = glm::translate(firstReflectModel, glm::vec3(0.0f, 0.5f, 0.0f));
glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),1, GL_FALSE, glm::value_ptr(firstReflectModel));
glDrawArrays(GL_TRIANGLES, 0, 36);
```

这里模板缓冲的作用，主要是使镜像看起来在镜面上，如果不使用模板缓冲，效果如下：

![没有使用模板的镜像](http://img.blog.csdn.net/20160814210534410)

这里红色部分显示在镜面外面，带有明显的视觉bug，通过模板缓冲，我们决定只将镜像在镜面部分的内容显示出来，形成的镜面效果看起来才更真实。下面给出一个完整的不使用和使用模板的效果对比图:

![效果对比](http://img.blog.csdn.net/20160814212041716)

在实现镜面效果的时候，注意镜面需要开启blend效果，就是混色的效果，通过混色，我们在镜面上显示了少许镜子的颜色(我们使用红色模拟镜面)，看到了更多的是物体的镜像的颜色。如果不开启混色，那么物体的镜像将被镜面遮住，看不到镜像。使用混色（blend）绘制镜面，可以通过如下代码实现：

```cpp
// section3 绘制反射平面
planeShader.use();
glBindVertexArray(planeVAOId);
glEnable(GL_BLEND); // 为反射平面启用混色
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDrawArrays(GL_TRIANGLES, 0, 6);
glDisable(GL_BLEND);
```
关于blend的内容，下一节将会介绍，这里不再展开。


# 最后的说明
模板测试使用的思路很简单，先通过绘制物体写入模板缓冲，这是一个建立的过程；第二步是利用模板缓冲中的值选择性地丢弃或者保留片元，从而制造特效。不同的效果，调弄这些模板函数的方式各不相同。在使用过程中，尤其需要注意的是何时开始缓冲区，以及缓冲区写入的模式设置问题。关于planar reflections，一个比较好的参考资料[点击这里](https://www.opengl.org/archives/resources/code/samples/mjktips/Reflect.html)。

# 参考资料
1.https://www.opengl.org/wiki/Stencil_Test
2.https://www.opengl.org/archives/resources/code/samples/mjktips/Reflect.html
3.http://www.cnblogs.com/mikewolf2002/archive/2012/05/15/2500867.html
4.https://open.gl/depthstencils
5.http://www.learnopengl.com/#!Advanced-OpenGL/Stencil-testing
6.https://community.arm.com/community/arm-cc-cn/blog/2015/03/19/stencil-test-%E6%A8%A1%E6%9D%BF%E6%B5%8B%E8%AF%95-%E4%B8%8D%E5%BE%97%E4%B8%8D%E8%AF%B4%E7%9A%84%E9%82%A3%E4%BA%9B%E4%BA%8B