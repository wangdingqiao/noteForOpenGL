Github公式无法渲染，可以到[我的博客](http://blog.csdn.net/wangdingqiaoit/article/details/51589825)阅读全文。

写在前面
    前面几节分别介绍了[模型变换](http://blog.csdn.net/wangdingqiaoit/article/details/51531002)，[视变换](http://blog.csdn.net/wangdingqiaoit/article/details/51570001)，本节继续学习OpenGL坐标变换过程中的投影变换。这里主要是从数学角度推导投影矩阵。**对数学不感兴趣的，可以稍微了解下，或者跳过本节内容。**
    
   **本文主要翻译并整理自 songho [OpenGL Projection Matrix](http://www.songho.ca/opengl/gl_projectionmatrix.html)一文**，这里对他的推导思路稍微进行了整理。

通过本节可以了解到

   - 透视投影矩阵的推导
   - 正交投影矩阵的 推导
   - 视口变换矩阵的推导
   - zFighting问题

## 投影变换
OpenGL最终的渲染设备是2D的，我们需要将3D表示的场景转换为最终的2D形式，前面使用模型变换和视变换将物体坐标转换到照相机坐标系后，需要进行投影变换，将坐标从相机---》裁剪坐标系，经过透视除法后，变换到规范化设备坐标系(NDC)，最后进行视口变换后，3D坐标才变换到屏幕上的2D坐标，这个过程如下图所示：

![坐标变换](http://img.blog.csdn.net/20140918202127843)

投影变换通过指定视见体(viewing frustum)来决定场景中哪些物体将可能会呈现在屏幕上。在视见体中的物体会出现在投影平面上，而在视见体之外的物体不会出现在投影平面上。投影包括很多类型，OpenGL中主要考虑透视投影([perspective projection](https://en.wikipedia.org/wiki/Perspective_(graphical)))和正交投影( [orthographic projection](https://en.wikipedia.org/wiki/Orthographic_projection_(geometry)))。两者之间存在很大的区别，如下图所示(图片来自[Modern OpenGL](https://glumpy.github.io/modern-gl.html))：

![投影类型](http://img.blog.csdn.net/20160605134913316)

上面的图中，红色和黄色球在视见体内，因而呈现在投影平面上，而绿色球在视见体外，没有在投影平面上成像。

指定视见体通过(GLdouble left,  GLdouble right,  GLdouble bottom,  GLdouble top,  GLdouble nearVal,  GLdouble farVal)6个参数来指定。注意在相机坐标系下，相机指向-z轴，nearVal和farVal表示的剪裁平面分别为:近裁剪平面$z = -nearVal$，以及远裁剪平面$z = -farVal$。推导投影矩阵，就要利用这6个参数。在OpenGL中成像是在近裁剪平面上完成。


## 透视投影矩阵的推导
透视投影中，相机坐标系中点被映射到一个标准立方体中，即规范化设备坐标系中，其中$[l,r]映射到[-1,1]$，$[b,t]$映射到[-1,1]中，以及$[n,f]$被映射到$[-1,1]$，如下图所示：
![视见体和NDC](http://img.blog.csdn.net/20140902190757269)

注意到上面的相机坐标系为右手系，而NDC中+z轴向内，为左手系。

### 我们的目标
求出投影矩阵的目标就是要找到一个透视投影矩阵P使得下式成立：
$$\begin{bmatrix} 
x_{c} \\ y_{c} \\ z_{c} \\ w_{c} 
\end{bmatrix} = P* \begin{bmatrix} 
x_{e} \\ y_{e} \\ z_{e} \\ w_{e} 
\end{bmatrix} $$
$$\begin{bmatrix} 
x_{n} \\ y_{n} \\ z_{n}
\end{bmatrix} = \begin{bmatrix} 
x_{c} / w_{c} \\ y_{c} / w_{c} \\ z_{c} / w_{c}
\end{bmatrix}$$
上面的除以$w_{clip}$过程被称为透视除法。要找到我们需要的矩阵P，我们需要利用两个关系:

+ 投影位置$x_{p}$,$y_{p}$和相机坐标系中点$x_{e}$,$y_{e}之间关系。投影后对于z分量都是$z_{p}=-nearVal$。
+ 利用$x_{p}$，$y_{p}$和$x_{ndc}，y_{ndc}$关系求出$x_{clip},y_{clip}$。
+ 利用$z_{n}$与$z_{e}$关系得出$z_{clip}$

#### 计算投影平面上的位置
投影时原先位于相机坐标系中的点$p=(x_{e},y_{e},z_{e})$投影到投影平面后，得到点$p'=(x_{p},y_{p},-nearVal)$。具体过程如下图所示：
![投影平面上的点](http://img.blog.csdn.net/20140902190859092)

需要空间想象一下，可以得出左边的图是俯视图，右边是侧视图。
利用三角形的相似性，通过俯视图可以计算得到:
$\frac{x_{p}}{x_{e}} = \frac{-n}{z_{e}}$
即: $x_{p} = \frac{x_{e}n}{-z_{e}} \tag{1.1}$
同理通过侧视图可以得到：
$y_{p} = \frac{y_{e}n}{-z_{e}}\tag{1.2}$

由(1)(2)这个式子可以发现，他们都除以了$-z_{e}$这个量，并且与之成反比。这可以作为透视除法的一个线索，因此我们的矩阵P的形式如下：
$$\begin{bmatrix} 
x_{c} \\ y_{c} \\ z_{c} \\ w_{c} 
\end{bmatrix} = \begin{bmatrix} 
. & . & .& . \\ 
. & . & .& . \\ 
. & . & .& . \\ 
0 & 0 & -1 & 0 \end{bmatrix} * \begin{bmatrix} 
x_{e} \\ y_{e} \\ z_{e} \\ w_{e} 
\end{bmatrix} $$
也就是说$w_{c} = -z_{e}$。
下面利用投影点和规范化设备坐标的关系计算出矩阵P的前面两行。
对于投影平面上$x_{p}$满足$[l,r]$线性映射到$[-1,1]$对于$y_{p}$满足$[b,t]$线性映射到$[-1,1]$。

其中$x_{p}$的映射关系如下图所示：

![投影点xp线性映射](http://img.blog.csdn.net/20140902192827502)

则可以得到$x_{p}$的线性关系：
$x_{n} = \frac{2}{r - l}x_{p}+\beta \tag{1.3}$
将(r,1)带入上式得到：
$\beta = -\frac{r+l}{r-l}$
带入式子3得到：
$x_{n} = \frac{2}{r - l}x_{p} -\frac{r+l}{r-l} \tag{1.4}$
将式子1带入式子5得到：
$$\begin{align}x_{n} &= \frac{2x_{e}n}{r - l}*\frac{1}{-z_{e}} -\frac{r+l}{r-l} \\
&= \frac{(\frac{2x_{e}n}{r - l}+\frac{r+l}{r-l}*z_{e})}{-z_{e}}
\end{align} \tag{1.5}$$

由式子6可以得到:
$x_{c} =\frac{2n}{r - l}x_{e}+\frac{r+l}{r-l}*z_{e} \tag{1.6} $

对于$y_{p}$的映射关系如下：
![投影点yp线性映射](http://img.blog.csdn.net/20160605145632561)
同理也可以计算得到:
$$\begin{align}y_{n} &= \frac{2y_{e}n}{t - b}*\frac{1}{-z_{e}} -\frac{t+b}{t-b} \\
&= \frac{(\frac{2y_{e}n}{t - b}+\frac{t+b}{t-b}*z_{e})}{-z_{e}}
\end{align} \tag{1.7}$$
$y_{c} =\frac{2n}{t - b}y_{e}+\frac{t+b}{t-b}*z_{e} \tag{1.8} $

由式子7和9可以得到矩阵P的前两行和第四行为：
$$\begin{bmatrix} 
x_{c} \\ y_{c} \\ z_{c} \\ w_{c} 
\end{bmatrix} = \begin{bmatrix} 
\frac{2n}{r-l} & 0 & \frac{r+l}{r-l} & 0 \\ 
0 & \frac{2n}{t-b} &\frac{t+b}{t-b} & 0 \\ 
. & . & .& . \\ 
0 & 0 & -1 & 0 \end{bmatrix} * \begin{bmatrix} 
x_{e} \\ y_{e} \\ z_{e} \\ w_{e} 
\end{bmatrix} $$

由于$z_{e}$投影到平面时结果都为$-n$，因此寻找$z_{n}$与之前的x,y分量不太一样。我们知道$z_{n}$与x,y分量无关，因此上述矩阵P可以书写为：
$$\begin{bmatrix} 
x_{c} \\ y_{c} \\ z_{c} \\ w_{c} 
\end{bmatrix} = \begin{bmatrix} 
\frac{2n}{r-l} & 0 & \frac{r+l}{r-l} & 0 \\ 
0 & \frac{2n}{t-b} &\frac{t+b}{t-b} & 0 \\ 
0& 0 & A & B \\ 
0 & 0 & -1 & 0 \end{bmatrix} * \begin{bmatrix} 
x_{e} \\ y_{e} \\ z_{e} \\ w_{e} 
\end{bmatrix} $$

则有：$z_{n} = \frac{Az_{e}+Bw_{e}}{-z_{e}}$,由于相机坐标系中$w_{e} = 1$，则可以进一步书写为：
$z_{n} = \frac{Az_{e}+B}{-z_{e}} \tag{1.9}$

要求出系数A,B则，**利用$z_{n}$与$z_{e}$的映射关系为：(-n，-1)和(-f,1)**，代入式子10得到：
$A =-\frac{f+n}{f-n}$和$B= -\frac{2fn}{f-n}$，
则$z_{n}$与$z_{e}$的关系式表示为：
$z_{n}=\frac{-\frac{f+n}{f-n}z_{e}-\frac{2fn}{f-n}}{-z_{e}}\tag{1.10}$
将A，B代入矩阵P得到：

$$P= \begin{bmatrix} 
\frac{2n}{r-l} & 0 & \frac{r+l}{r-l} & 0 \\ 
0 & \frac{2n}{t-b} &\frac{t+b}{t-b} & 0 \\ 
0& 0 & \frac{-(f+n)}{f-n} & \frac{-2fn}{f-n} \\ 
0 & 0 & -1 & 0 \end{bmatrix} \tag{透视投影矩阵}$$

上述矩阵时一般的视见体矩阵，**如果视见体是对称的**，即满足$r=-l,t=-b$，则矩阵P可以简化为：
$$P= \begin{bmatrix} 
\frac{n}{r} & 0 & 0 & 0 \\ 
0 & \frac{n}{t} & 0 & 0 \\ 
0& 0 & \frac{-(f+n)}{f-n} & \frac{-2fn}{f-n} \\ 
0 & 0 & -1 & 0 \end{bmatrix} \tag{简化的透视投影矩阵}$$

### 使用Fov指定的透视投影
另外一种经常使用 的方式是通过视角(Fov)，宽高比(Aspect)来指定透视投影，例如旧版中函数[gluPerspective](https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml)，参数形式为：
> API void gluPerspective(GLdouble fovy,  GLdouble aspect,  GLdouble zNear,  GLdouble zFar);

其中指定fovy指定视角，aspect指定宽高比，zNear和zFar指定剪裁平面。fovy的理解如下图所示(来自[opengl 投影](http://www.gimoo.net/t/1507/55a62113251f2.html))：
![fov](http://img.blog.csdn.net/20160606093332723)

这些参数指定的是一个对称的视见体，如下图所示(图片来自[Working with 3D Environment](http://www.codersource.net/2011/01/22/working-with-3d-environment-opengl-tutorial/))：
![perspective](http://img.blog.csdn.net/20160606093500084)

由这些参数，可以得到：
$h = near*tan(\frac{\theta}{2})$
$w=h*aspect$
对应上述透视投影矩阵中：
$r=-l,r=w$
$t=-b, t=h$
则得到透视投影矩阵为：
$$P= \begin{bmatrix} 
\frac{cot(\frac{\theta}{2})}{aspect} & 0 & 0 & 0 \\ 
0 & cot(\frac{\theta}{2}) & 0 & 0 \\ 
0& 0 & \frac{-(f+n)}{f-n} & \frac{-2fn}{f-n} \\ 
0 & 0 & -1 & 0 \end{bmatrix} \tag{Fov透视投影矩阵}$$
## 正交投影矩阵的推导
相比于透视投影，正交投影矩阵的推导要简单些，如下图所示：
![正交投影](http://img.blog.csdn.net/20140902201217218)

对于正交投影，有$x_{p}=x_{e},y_{p}=y_{e}$，因而可以直接利用$x_{e}$与$x_{n}$的映射关系：$[l,-1],[r,1]$，利用$y_{e}$和$y_{n}$的映射关系：$[b,-1],[t,1]$，以及$z_{e}$和$z_{n}$的映射关系：$[-n,-1],[-f,1]$。例如$x_{e}$与$x_{n}$的映射关系表示为如下图所示：

![x分量的映射关系](http://img.blog.csdn.net/20160605162101578)

利用$[l,-1],[r,1]$得到:

$x_{n} = \frac{2}{r - l}x_{e} - \frac{r + l}{r - l} \tag{2.1}$
同理可得到y,z分量的关系式为：
$y_{n} = \frac{2}{t - b}y_{e} - \frac{t + b}{t - b} \tag{2.2}$
$z_{n} = \frac{-2}{f-n}z_{e}-\frac{f + n}{f - n} \tag{2.3}$

对于正交投影而言，w成分是不必要的，保持为1即可，则所求投影矩阵第四行为(0,0,0,1)，w保持为1，则NDC坐标和剪裁坐标相同，从而得到正交投影矩阵为:
$$ O= \begin{bmatrix} \frac{2}{r-l} & 0 & 0 & -\frac{r+l}{r-l} \\ 
0 & \frac{2}{t - b} & 0 & -\frac{t+b}{t-b} \\
0 & 0 & \frac{-2}{f - n} & -\frac{f+n}{f - n} \\
0 & 0 & 0 & 1
\end{bmatrix} \tag{正交投影矩阵}$$

如果视见体是对称的，即满足$r=-l,t=-b$，则矩阵O可以简化为：
$$ O= \begin{bmatrix} \frac{1}{r} & 0 & 0 & 0 \\ 
0 & \frac{1}{t} & 0 & 0 \\
0 & 0 & \frac{-2}{f - n} & -\frac{f+n}{f - n} \\
0 & 0 & 0 & 1
\end{bmatrix} \tag{简化正交投影矩阵}$$

### 利用平移和旋转推导正交投影矩阵
还可以看做把视见体的中心移动到规范视见体的中心即原点处，然后缩放视见体使得它的每条边长度都为2，进行这一过程的变换表示为：
$$\begin{align} O &=S(2/(r-l),2/(t-b),2/(near - far))*T(-(r+l)/2, -(t+b)/2, (f+n)/2) \\
&=\begin{bmatrix} \frac{2}{r-l} & 0 & 0 & 0 \\
0 & \frac{2}{t-b} & 0 & 0 \\
0 & 0 & \frac{2}{n - f} & 0 \\
0 & 0 & 0 & 1
\end{bmatrix} * 
 \begin{bmatrix} 1 & 0 & 0 & -\frac{r+l}{2} \\
0 & 1 & 0 & -\frac{t+b}{2} \\
0 & 0 & 1 & \frac{f+n}{2} \\
0 & 0 & 0 & 1
\end{bmatrix} \\
&= \begin{bmatrix}
\frac{2}{r-l} & 0 & 0 & -\frac{r+l}{r-l} \\
0 & \frac{2}{t - b} & 0 & -\frac{t+b}{t-b} \\
0 & 0 & \frac{-2}{f-n} & -\frac{f+n}{f-n} \\
0 & 0 & 0 & 1
 \end{bmatrix}
\end{align}$$

## 视口变换矩阵的推导
视变换是将NDC坐标转换为显示屏幕坐标的过程，如下图所示：

![视口变换](http://img.blog.csdn.net/20160605173813298)
视口变化通过函数:
[glViewport(GLint  $s_{x}$ , GLint $s_{y}$ , GLsizei $w_{s}$ , GLsizei  $h_{s}$)](https://www.opengl.org/sdk/docs/man/html/glViewport.xhtml);
[glDepthRangef(GLclampf $n_{s}$ , GLclampf $f_{s}$ )](https://www.khronos.org/opengles/sdk/docs/man/xhtml/glDepthRangef.xml);

两个函数来指定。其中($s_{x}$,$s_{y}$)表示窗口的左下角，$n_{s}$和 $f_{s}$指定远近剪裁平面到屏幕坐标的映射关系。
使用线性映射关系如下：

$(-1,s_{x}),(1,s_{x}+w_{s}) \tag{x分量映射关系}$
$(-1,s_{y}),(1,s_{y}+h_{s}) \tag{y分量映射关系}$
$(-1,n_{s}),(1,f_{s}) \tag{z分量映射关系}$

求出线性映射函数为:
$x_{s}=\frac{w_{s}}{2}x_{n}+s_{x}+\frac{w_{s}}{2} \tag{3.1}$
$y_{s} = \frac{h_{s}}{2}y_{n}+s_{y}+\frac{h_{s}}{2} \tag{3.2}$
$z_{s}= \frac{f_{s}-n_{s}}{2}z_{n}+\frac{n_{s} + f_{s}}{2} \tag{3.3}$
则由上述式子得到视口变换矩阵为：
$$viewPort=\begin{bmatrix} 
\frac{w_{s}}{2} & 0 & 0 & s_{x}+\frac{w_{s}}{2} \\
0 &  \frac{h_{s}}{2} & 0 & s_{y}+\frac{h_{s}}{2} \\
0 & 0 & \frac{f_{s}-n_{s}}{2} & \frac{n_{s} + f_{s}}{2} \\
0 & 0 & 0 & 1
\end{bmatrix} \tag{视口变换矩阵}$$

##Zfighting问题
回过头去看透视投影部分，$z_{n}$与$z_{e}$的关系式1.10：
$z_{n}=\frac{-\frac{f+n}{f-n}z_{e}-\frac{2fn}{f-n}}{-z_{e}}\tag{1.10}$
这是一个非线性关系函数，作图如下：
![zfighting](http://img.blog.csdn.net/20160606100958977)
从左边图我们可以看到，在近裁剪平面附近$z_{n}$值变化比较大，精确度较好；而在远裁剪平面附近，有一段距离内，$z_{n}$近乎持平，精确度不好。当增大远近裁剪平面的范围$[-n,-f]$后，如右边图所示，我们看到在远裁剪平面附近，不同相机坐标$z_{e}$对应的$z_{n}$相同，精确度低的现象更为明显，这种深度的精确度引起的问题称之为zFighting。要尽量减小[-n,-f]的范围，以减轻zFighting问题。


## 本节参考资料
1. songho [OpenGL Projection Matrix](http://www.songho.ca/opengl/gl_projectionmatrix.html)
2. [GLSL Programming/Vertex Transformations](https://en.wikibooks.org/wiki/GLSL_Programming/Vertex_Transformations)
3. [glOrtho ](https://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml)
4. [glFrustum](https://www.opengl.org/sdk/docs/man2/xhtml/glFrustum.xml)
5. [gluPerspective](https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml)

## 相关资源
1.[The Perspective and Orthographic Projection Matrix](http://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix)
2.[OpenGL 101: Matrices - projection, view, model ](https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/)
3.[Calculating the gluPerspective matrix and other OpenGL matrix maths](https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/)