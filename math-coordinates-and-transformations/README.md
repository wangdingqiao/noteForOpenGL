公式部分无法显示，请转到[CSDN博客](http://blog.csdn.net/wangdingqiaoit/article/details/51394238) 阅读本文。
写在前面
        上一节介绍了[向量和矩阵](http://blog.csdn.net/wangdingqiaoit/article/details/51383052)，本节将熟悉坐标、线性变换、仿射变换以及坐标转换等概念和计算方法，这些内容对后续的学习将会有很大帮助。**部分内容不是OpenGL编程初学者所必须掌握的，可以在以后需要时再回头来看。**

这里是对这些知识点的一个总结，旨在对他们有个整体把握，后面具体应用时会使用这些概念。内容尽量以例子形式说明，仅在必要时会给出数学证明。**一个主题往往涉及过多内容，对于文中省略的部分，请参考相应的教材。**

通过本节可以了解到

- 坐标的概念
- 线性变换的概念和计算方法
- 仿射变换
- 坐标转换的概念和计算方法

## 坐标系及坐标
坐标是在特定坐标系下表示物体位置的方法，一谈到坐标，必定是在某个坐标系下给定的。例如经纬度坐标是相对于地球的球面坐标系统给出的。

坐标系则给出了一个参考框架，在这个框架里面，定义其他位置相对于一个起始点（这个起始点称作原点$O$）的位置。同一位置，在不同的坐标系下会有不同的坐标，例如你所在城市以经纬度坐标表示，是相对于地球坐标系来给定的，如果从太阳系或者银河系来给定，又会是另外一个不同的值。
常见的坐标系包括:笛卡尔坐标系，极坐标系，球面坐标系等。
如下图所示的我们熟悉的，2D笛卡尔坐标系(来自[wiki](https://en.wikipedia.org/wiki/Cartesian_coordinate_system)):
![这里写图片描述](http://img.blog.csdn.net/20160513113230319)

如下图所示的熟悉的3D笛卡尔坐标系(来自[wiki](https://en.wikipedia.org/wiki/Cartesian_coordinate_system)):
![这里写图片描述](http://img.blog.csdn.net/20160513113354624)

### 定义坐标系
定义一个坐标系需要指定(参考自[Objects in Motion](http://alfonse.bitbucket.org/oldtut/Positioning/Tutorial%2006.html)):

- 坐标系的维度 2D, 3D, 4D等等
- 定义坐标空间轴的向量 这些向量成为基向量，他们有名字，例如x,y,z;这些向量一般而言都是正交的，但不一定非得互相正交(只要线性无关即可，后面介绍)，但是每一个维度必须只有一个轴。
- 坐标系的原点O 原点是导出其他点的参考点。
- 一个表明空间中点是否合法的区域 在此区域之外的点就不再合法。这个区域根据空间不同，可以是无穷的。

这里，维度已由基向量维数确定，合法区域一般是无穷的，但是在图形处理中某些坐标空间是有限的，例如规范化设备空间（后面其他文章会介绍）。作为一个了解，基向量不一定非得正交，如下图所示:
![非正交的轴](http://img.blog.csdn.net/20160513113855485)

对于一般情况，我们只需要记住:
**坐标系=(基向量，原点$O$)**

### 左手坐标系和右手坐标系
对于任意2个2D坐标系，我们通过旋转、移动翻转可以将两个坐标系xy轴的指向相同。

但是对于3D坐标系，任意两个坐标系却不能等价。实际上，存在两种完全不同的3D坐标系：左手坐标系和右手坐标系。如果同属于左手坐标系或者右手坐标系，则可以通过旋转来重合，否则不可以。
判断一个坐标系是否属于右手系，可以拿出右手，然后右手的大拇指代表+x轴指向，食指代表+y轴指向，中指代表+z轴指向，你可以转动你的右手来匹配这个坐标系，如果能匹配则是右手坐标系，否则是左手坐标系。判断左手坐标系的方法类似。如下图所示为判断右手坐标系的方法(来自[OpenGL coordinate system](https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/)):
![这里写图片描述](http://img.blog.csdn.net/20160513120345751)

关于左右手坐标系理解还可以可参考下图(来自《3D数学基础》):
![左手右手坐标系](http://img.blog.csdn.net/20140917210007203)
如上右图右手坐标系，这里拇指、食指、其余手指分别代表x,y,z轴的正方向。高等数学教材上使用的是右手坐标系。

### 旋转正方向的判断
同样还存在一个左手规则和右手规则，可以用于判断当物体绕轴旋转时正方向的判定问题。

对于左手规则，确定一个旋转轴后，左手握成拳头，拇指指向旋转轴的正方向，则其余手指弯曲方向即为旋转的正方向。从旋转轴正向末端来看，正向旋转是顺时针的。对于右手规则，有同样的方法。可参见下图：
![左手右手规则](http://img.blog.csdn.net/20140917210141436)

左手右手规则在不同场合有着不同作用。[上一节]((http://blog.csdn.net/wangdingqiaoit/article/details/51383052))，我们使用右手规则判断了叉积的结果向量的方向。

**注意OpenGL中坐标系**  OpenGL中的物体、世界、照相机坐标系都属于右手坐标系，而规范化设备坐标系使用左手坐标系。笼统地说OpenGL使用右手坐标系是不合适的。这些坐标系后面会介绍。关于这个问题可以参考[SO](http://stackoverflow.com/questions/4124041/is-opengl-coordinate-system-left-handed-or-right-handed).

### 坐标
坐标是在指定坐标系中，相对于原点$O$给出的一个位置。这个位置可以用有序实数对表示(有些坐标系中可能使用复数)，注意数对中的数字顺序对结果有影响。上面提到**坐标系=(基向量，原点$O$)**，3D坐标系用向量表示为:
$\begin{cases} i=(a_{x}, b_{y}, b_{z}) \\ j = (b_{x}, b_{y},b_{z}) \\ k =(c_{x},c_{y},c_{z})\end{cases} \tag{基向量}$
$O=(O_{x},O_{y},O_{z})\tag{坐标原点}$
这样在坐标系中一点$P$与原点$O$构成的向量:
$r=\vec{OP}=xi+yj+zk$
这时称$(x,y,z)$为点$P$的坐标，这个坐标也可以表示向量$\vec{OP}$.
一般地使用的3D笛卡尔坐标系使用标准基向量和坐标原点：
$\begin{cases} i=(1, 0, 0) \\ j = (0, 1,0) \\ k =(0,0,1)\end{cases} \tag{标准基向量}$
$O=(0,0,0)\tag{标准坐标原点}$.

 从上面可以看到，在一个坐标系中，求取坐标的过程，是一个向量分解的过程。求取一个位置在另一个不同的坐标系中的坐标，则需要进行坐标转换。后面会介绍。
 
### 使用坐标系的优势
使用坐标系统便能以解析几何的形式来研究空间几何。通过建立一个坐标系使得空间中点用有序实数组表示，空间图形用方程表示，这样能方便地研究几何图形的性质。

----

## 必要基础概念
了解线性变换、仿射变换以及坐标转换，对于后面学习图形编程中的模型变换方法有很大帮助，因此这里予以介绍。要了解这些概念，需要一些其他概念的支撑，这里逐一介绍。**每一个概念以定义结合示例的形式给出，如果暂时没有理解清楚，可以暂时跳过，以后回过头来再看或者参考这个主题的其他资料。**本部分最后会给出完整的线性变换示例。

### 向量组的线性组合
向量组是一组向量的集合，例如$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$表示一个由m个$n \times 1$的矩阵(n维列向量)组成的列向量组。对应的也有行向量组的概念。如果存在一组实数$\lambda_{1}, \lambda_{2},\cdots,\lambda_{m}$，使得向量$\beta$满足下式:
$\beta = \lambda_{1} \alpha_{1} + \lambda_{2} \alpha_{2} + \cdots+\lambda_{m} \alpha_{m} \tag{1}$

则称向量$\beta$是$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$的线性组合，或者说$\beta$由$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$线性表示。上述求解点$P$坐标时向量$r$分解为向量组$i,j,k$表示的过程，就是一个找出线性表示系数的过程。

### 向量组线性无关
> 对于向量组$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$，如果存在不全为零的数$\lambda_{1}, \lambda_{2},\cdots,\lambda_{m}$，使下面的等式成立:
$\lambda_{1} \alpha_{1} + \lambda_{2} \alpha_{2} + \cdots+\lambda_{m} \alpha_{m} = 0 \tag{2}$
则称向量组$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$线性相关(linearly dependent), 否则称为线性无关( linearly independent)。也就是要使向量组$\alpha_{1},\alpha_{2},\cdots,\alpha_{m}$线性无关，那么所有的系数$\lambda_{i}$都必须为0。

**线性相关的一种几何解释 来自[math.stackexchange](http://math.stackexchange.com/questions/456002/what-exactly-does-linear-dependence-and-linear-independence-imply)**:
假定你有一组向量$\{{x_{1},x_{2},\cdots,x_{5}}\}$，你从某个点出发，沿着$x_{1}$走动一段距离，然后沿着$x_{2}$走动一段距离, 最后沿着$x_{5}$走动一段距离，最终你又回到了出发点（这里表明存在$\lambda_{1} x_{1} + \lambda_{2} x_{2} +\lambda_{5} x_{5} = 0 $,即式2成立）。这就说明$\{{x_{1},x_{2},\cdots,x_{5}}\}$是线性相关的。

当然存在其他方法，判断向量组线性相关性，感兴趣的可以参考线性代数教材。

### 线性空间的基
向量空间(也叫做线性空间)是对我们经常使用的2D和3D空间一般规律的拓展，它的定义主要反映的是满足一系列的运算规律，例如交换律和结合律等。由于这个定义包含较多规则，不在此列出，感兴趣的可以参考[vector space](https://en.wikipedia.org/wiki/Vector_space)。

> 如果在线性空间V中存在n个**线性无关**的向量$\alpha_{1},\alpha_{2},\cdots,\alpha_{n}$使得V中任意元素$\alpha$都能由他们线性表示，则称$\alpha_{1},\alpha_{2},\cdots,\alpha_{n}$为V的一个**基**。基所含向量个数n称为线性空间V的维数，并称V为n维线性空间。

例如2D空间中，二维向量组 $i=(1,0), j=(0,1)$是它的一个基；在3D空间中，向量组: $i=(1, 0, 0) , j = (0, 1,0),k =(0,0,1)$是它的一个基。类似的可以推广到n维向量空间的基。

### 向量在基下的坐标
> 设$\alpha_{1},\alpha_{2},\cdots,\alpha_{n}$是n维线性空间V的一个基，若任取$\alpha \in V$，总有且仅有一组有序实数$x_{1},x_{2},\cdots, x_{n}$，使得:
$\alpha = x_{1} \alpha_{1} + x_{2} \alpha_{2} + \cdots + x_{n} \alpha_{n}=(\alpha_{1}, \alpha_{2}, \cdots,\alpha_{n}) \begin{bmatrix} x_{1} \\ x_{2} \\ \vdots \\ x_{n}\end{bmatrix} \tag{3}$
成立，则称这组有序数$x_{1},x_{2},\cdots, x_{n}$为元素$\alpha $在基$\alpha_{1},\alpha_{2},\cdots,\alpha_{n}$下的**坐标**，记作$(x_{1},x_{2},\cdots, x_{n})^{T}$。

这里元素$\alpha$用基向量组的线性组合来表示，坐标就是线性组合的系数。例如向量$a=(3,4)=3e_{1}+4e_{2}$，其中$e_{1}=(1,0),e_{2}=(0,1)$为标准基，则a的坐标为$(3,4)^{T}$。

----

## 线性变换(Linear Transformations)
变换这个词类似于函数，即将一个定义域里的输入量转化为值域里的另一个值，变换就是一个映射关系，一种规则。线性变换的一些性质对于后续学习3D模型变换时，理解起来将会更容易。

> **线性变换** $T:U\rightarrow V$是一个函数，将定义域U中元素，映射到值域V中，并满足下列两个条件(参考[Definition LT](http://linear.ups.edu/html/section-LT.html)):
1)可加性 对任意$u_{1}, u_{2} \in U$，都满足: $T(u_{1} + u_{2}) = T(u_{1}) + T(u_{2})$
2)齐次性 对任意$u \in U$和任意标量$k$，都满足: $T(ku)=kT(u)$

可以利用上面的两个条件，即可加性和齐次性条件验证一个变换是否是线性变换。
**线性变换示例([来自Example ALT](http://linear.ups.edu/html/section-LT.html))**
$T(\begin{bmatrix} x_{1} \\ x_{2} \\ x_{3} \end{bmatrix})=\begin{bmatrix} 2x_{1}+x_{3} \\ -4x_{2}\end{bmatrix}$
首先验证其是否满足可加性:
$$\begin{align} T(x+y) &=T(\begin{bmatrix} x_{1} \\ x_{2} \\ x_{3} \end{bmatrix}+\begin{bmatrix} y_{1} \\ y_{2} \\ y_{3} \end{bmatrix}) \\
&=(\begin{bmatrix} x_{1}+y_{1} \\ x_{2}+y_{2} \\ x_{3}+y_{3} \end{bmatrix})\\
&= \begin{bmatrix} 2(x_{1}+y_{1}) +(x_{3}+y_{3}) \\ -4(x_{2}+y_{2})\end{bmatrix} \\
&= \begin{bmatrix} 2x_{1} + x_{3} \\ -4x_{2}\end{bmatrix} + \begin{bmatrix} 2y_{1}+y_{3} \\ -4y_{2}\end{bmatrix} \\
&= T(\begin{bmatrix} x_{1} \\ x_{2} \\ x_{3} \end{bmatrix}) + T(\begin{bmatrix} y_{1} \\ y_{2} \\ y_{3} \end{bmatrix}) \\
& = T(x) + T(y)
 \end{align} \tag{*}$$

然后验证是否满足齐次性:
$$\begin{align} T(kx) &=T(k\begin{bmatrix} x_{1} \\ x_{2} \\ x_{3} \end{bmatrix}) \\
&=T(\begin{bmatrix} kx_{1} \\ kx_{2}\\ kx_{3} \end{bmatrix})\\
&= \begin{bmatrix} 2(kx_{1}) +(kx_{3}) \\ -4(kx_{2})\end{bmatrix} \\
&= k\begin{bmatrix} 2x_{1}+x_{3} \\ -4x_{2}\end{bmatrix} \\
&= kT(\begin{bmatrix} x_{1} \\ x_{2} \\ x_{3} \end{bmatrix})\\
&= kT(x)
 \end{align} \tag{**}$$
由上述\*和\**式可知，变换T满足可加性和齐次性，因而是线性变换。
实际上判断线性变换还有一个[简单方法](http://mathinsight.org/linear_transformation_definition_euclidean)，即看变换后每个组成部分是不是原来分量的倍数（不能为常量）。
例如变换$f(x,y,z) = (3x-y, 3z, 0, z-2x)$是线性变换，而:
$g(x,y,z) = (3x-y, 3z+2, 0,z-2x)$和
$h(x,y,z) = (3x-y, 3xz, 0,z-2x)$
都不是线性变换。其中g中$3z+2$的部分2是个常量，不满足倍数要求；h中$3xz$不满足倍数要求，因而也不是线性变换。
对于线性变换一个重要的性质是(令齐次性表达式中u=0可得到):
 $T(0)=0 \tag{1}$
 
### 线性变换与矩阵一一对应
> 对一个线性变换$T$，存在一个矩阵$A$与之对应，变换表示为$T(x)=Ax$，其中$x$为列向量。

证明: 1.首先证明充分性。
        当$T$是线性变换时，对于$x \in U$，其中 $$ x=\begin{bmatrix} 
        x_{1} \\ x_{2} \\ \vdots \\ x_{n} \end{bmatrix}$$，有:
        $$\begin{align} 
        T(x) &=T(x_{1}e_{1}+x_{2}e_{2}+ \cdots + x_{n}e_{n}) \\
                & = x_{1}T(e_{1})+x_{2}T(e_{2})+ \cdots + x_{n}T(e_{n}) \\
                &= (T(e_{1}), T(e_{2}), \cdots, T(e_{n}))\begin{bmatrix} 
        x_{1} \\ x_{2} \\ \vdots \\ x_{n} \end{bmatrix} \\
               &= Ax 
    \end{align}$$
其中:
$A=(T(e_{1}), T(e_{2}), \cdots, T(e_{n}))$称为标准矩阵，它的每一列都是标准基$e_{i}$变换后的向量在原基下的表示。
2.证明必要性
当变换$T$对应矩阵$A$，表示为$T(x)=Ax$时，证明$T$为线性变换。
由矩阵乘法性质，容易得到:
$T(x+y)=A(x+y)=Ax+Ay = T(x)+T(y) \tag{矩阵乘法分配律}$
$T(kx)=A(kx)=kA(x)=kT(x)\tag{矩阵标量乘法法则}$
则说明$T$满足可加性和齐次性，因而是一个线性变换。
由1,2得到，线性变换$T$总是与一个矩阵$A$一一对应。

例如矩阵$A$如下(下面的例子来自:[Matrices and linear transformations](http://mathinsight.org/matrices_linear_transformations)):
$\begin{align*}
  A = \left[
    \begin{array}{rrr}
      1 & 0 & -1\\
      3 & 1 & 2
    \end{array}
  \right].
\end{align*}$
对应的线性变换为:
$T(x)=\begin{align*}
  Ax = 
  \left[
    \begin{array}{rrr}
      1 & 0 & -1\\
      3 & 1 & 2
    \end{array}
  \right]
  \left[
    \begin{array}{c}
      x\\
      y\\
      z
    \end{array}
  \right]
  =
  \left[
    \begin{array}{c}
      x - z\\
      3x + y +2z
    \end{array}
  \right]
  =(x-z,3x+y+2z).
\end{align*}$
由给定的线性变换:
$f(x,y)=(2x+y,y,x-3y)$
要找到对应的矩阵A，由上面定理证明时可知，标准矩阵A的内容为:
$A=(f(e_{1}),f(e_{2})))$
其中$f(e_{1})$计算如下:
$\begin{align*}
  f(e_{1})=f(1,0)=(2,0,1) = 
  \left[
    \begin{array}{r}
      2\\0\\1
    \end{array}
  \right].
\end{align*}$
$f(e_{2})$计算如下:
$\begin{align*}
  f(e_{2})=f(0,1) = (1,1,-3) = 
  \left[
    \begin{array}{r}
      1\\1\\-3
    \end{array}
  \right].
\end{align*}$
所以最终计算得到矩阵$A$如下:
$\begin{align*}
  A=
  \left[
    \begin{array}{rr}
      2 & 1\\
      0 & 1\\
      1 & -3
   \end{array}
  \right].
\end{align*}$

后面会详细介绍矩阵A的计算方法。在此之前，先来看下线性变换的应用。

### 线性变换的应用
线性变换在3D图形中模型变换部分应用很多，例如旋转、错切(shear)、缩放等操作都是线性变换。在OpenGL中使用矩阵操作来表示这些线性变换。例如2D平面上绕原点的旋转$\theta$角度的操作如下图所示:
![旋转](http://img.blog.csdn.net/20160517222229262)

通过**利用极坐标系表示$x,y和x',y'$并利用三角函数公式**（具体过程略,有兴趣可以自行推导，后面会介绍另外一个更简洁的方法），得到旋转矩阵为:
$\begin{bmatrix} x' \\ y' \end{bmatrix}=\begin{bmatrix} cos\theta & -sin\theta  \\ sin\theta & cos\theta \end{bmatrix}\begin{bmatrix} x \\ y\end{bmatrix}$
那么矩阵:
$A(\theta)=\begin{bmatrix} cos\theta & -sin\theta  \\ sin\theta & cos\theta \end{bmatrix}$表示的就是这个围绕原点旋转$\theta$角度的线性变换。

**使用矩阵表达变换的优势** 通过使用矩阵形式，能够级联对物体顶点的变换，例如先执行旋转，后执行缩放，则表示为:$T(x)=T_{s}T_{r}x=(T_{s}T_{R})x$，可以在对描述物体组成的多个顶点执行变换之前，先计算出矩阵$(T_{s}T_{R})$，从而节省执行变换需要的时间；同时也能表示为逆操作，例如旋转$-\theta$角度，$T(x)=A(-\theta)x$。只是在OpenGL中使用的标准矩阵$A$是4x4的，后面会介绍原因和具体构造方法。

### 线性变换对应矩阵的计算方法(重要结论)

**线性变换由基及变换后基的值唯一确定，通过计算线性变换后基的值可以得到线性变换对应的矩阵A。** 这是本节讲述线性变换最重要的结论。

> **定理**: 设$u_{1}, u_{2}, \cdots, u_{n}$是线性空间U的一个基，线性空间V包含向量$v_{1}, v_{2}, \cdots, v_{n}$(可以相同)。那么存在唯一的线性变换$T:U \rightarrow V$使得:$T(u_{i})=v_{i}(1 \leq i \leq n)$。

上面的定理证明，感兴趣的可以参考[ Theorem LTDB](http://linear.ups.edu/html/section-LT.html)，不在此处给出详细过程。

这个定理告诉我们: 只要知道了线性空间U的给定基$u_{1}, u_{2}, \cdots, u_{n}$在线性变换$T$下对应的值$T(u_{1}), T(u_{2}), \cdots, T(u_{n})$，线性变换$T$也就由$u_{i}$及$T(u_{i})$的对应关系确定了。
这个定理的作用就好比，两点确定一条直线。因此我们可以通过计算:
$A=(T(u_{1}),T(u_{2}), \cdots, T(u_{n}))$来获取线性变换$T$对应的矩阵$A$。

也就是说，矩阵$A$的列向量，由$T(u_{i})$在基$u_{1}, u_{2}, \cdots, u_{n}$下的坐标唯一确定。同时给出一个矩阵$A$作为线性变换$T$在基$u_{1}, u_{2}, \cdots, u_{n}$下的矩阵，也就给出了该基在线性变换$T$下对应的值$T(u_{1}),T(u_{2}), \cdots, T(u_{n})$，从而确定了线性变换$T$。这表明线性变换$T$与矩阵之间存在一一对应关系，上面已经证明了这个结论。

例如对于上面的旋转矩阵，从基和转换后基的角度，也就是从x,y轴来看，$x$转换后为$x'$，$y$转换后为$y'$，如下图所示([doitpoms.ac.uk](http://www.doitpoms.ac.uk/tlplib/tensors/images/rotation.gif)):。
![这里写图片描述](http://img.blog.csdn.net/20160518092833432)
利用三角函数公式可以计算转换后$x',y'$对应的坐标，如下图所示(来自:[Rotations and Infinitesimal Generators](https://reedbeta.wordpress.com/2011/09/18/rotations-and-infinitesimal-generators/)):
![这里写图片描述](http://img.blog.csdn.net/20160518093100717)。
因此可以顺利的写出旋转矩阵为:
$R(\theta)= \begin{bmatrix} cos\theta & -sin\theta \\ sin\theta & cos\theta \end{bmatrix}$。
矩阵第一列即为$x$转换后的$x'$轴的对应的坐标，第二列即为$y$转换后的$y'$轴对应的坐标。显然这个计算方法，比利用极坐标公式来得快。
在后面使用OpenGL模型变换矩阵时，经常要使用到这个方法，理解了这一点后面理解模型变换矩阵就会变得简单。

---

## 基变换与坐标变换(重要结论)

一个位置，在不同的坐标系里有不同的坐标。设$u=\{u_{1},u_{2},\cdots,u_{n}\}$和$v=\{ v_{1},v_{2},\cdots, v_{n}\}$是n维线性空间V的两个基。两个基之间可以互相表示。因此存在9个标量$\lambda_{ij}$，使得:
$u_{1}=\lambda_{11}v_{1} + \lambda_{12}v_{2}+\lambda_{13}v_{3}$
$u_{2}=\lambda_{21}v_{1} + \lambda_{22}v_{2}+\lambda_{23}v_{3}$
$u_{3}=\lambda_{31}v_{1} + \lambda_{32}v_{2}+\lambda_{33}v_{3}$
$\lambda_{ij}$写成矩阵形式得到:
$$M=\begin{bmatrix} \lambda_{11} & \lambda_{12} & \lambda_{13} \\ \lambda_{21} & \lambda_{22} & \lambda_{23} \\ \lambda_{31} & \lambda_{32} & \lambda_{33} \\
\end{bmatrix}$$
则上面的式子可以简写为:
$u=Mv \tag{a}$
矩阵$M$称为**过渡矩阵**，通过矩阵$M$我们可以把向量在基u下的表示转换为基v下的表示，通过$M^{-1}$可以把向量在v下的表示转换为u下的表示。
设有一个向量$w$，它在基u和v下的表示分别为:
$$\begin{align} w &=a_{1}v_{1} + a_{2}v_{2} + \cdots +a_{n}v_{n} =a^{T}v\\
&= b_{1}u_{1} + b_{2}u_{2} + \cdots +b_{n}u_{n} =b^{T}u \end{align} $$
其中:
$$a=\begin {bmatrix} a_{1} \\ a_{2} \\ \vdots \\ a_{n} \end{bmatrix}$$
$$b=\begin {bmatrix} b_{1} \\ b_{2} \\ \vdots \\ b_{n} \end{bmatrix}$$
由上式得到:
$a^{T}v =b^{T}u \tag{b}$

将式子a代入式子b得到:
$a=M^{T}b \tag{c}$
$b=(M^{T})^{-1}a=Ta \tag{d}$

式子c和d给出了向量$w$的两个坐标之间的转换公式。
上面的矩阵$M^{T}$：
$$M^{T}=\begin{bmatrix} \lambda_{11} & \lambda_{21} & \lambda_{31} \\ \lambda_{12} & \lambda_{22} & \lambda_{32} \\ \lambda_{13} & \lambda_{23} & \lambda_{33} \\
\end{bmatrix}$$
这个矩阵的三个列向量，就是基$u=\{u_{1},u_{2},\cdots,u_{n}\}$的向量在基v下的坐标表示，即:
$M^{T}=([u_{1}]_{v}, [u_{2}]_{v},\cdots, [u_{n}]_{v})$
对应的$(M^{T})^{-1}$矩阵即是$v=\{ v_{1},v_{2},\cdots, v_{n}\}$在u下的坐标表示。
$(M^{T})^{-1}=([v_{1}]_{u}, [v_{2}]_{u},\cdots, [v_{n}]_{u})$.
那么上面的式子c和d可以分别改写为:
$[\omega]_{v}=[u]_{v}[\omega]_{u} \tag{e}$
$[\omega]_{u}=[v]_{u}[\omega]_{v} \tag{f}$

从式子e和f来看，我们可以得出，要在两个基之间转换坐标，只需要求出一个基在另一个基里的坐标表示即可，这个坐标表示构成矩阵$M^{T}$或者$(M^{T})^{-1}$。

**示例**
已知基$u$为:
$u_{1}=(1,2,1,0), u_{2}=(3,3,3,0), u_{3}=(2,-10,0,0), u_{4}=(-2,1,-6,2)$
基$v$为:
$v_{1}=(1,2,1,0),v_{2}=(1,-1,1,0), v_{3}=(1,0,-1,0),v_{4}=(0,0,0,2)$
容易求出:
$v_{1}=u_{1}$
$v_{2}=-2u_{1}+u_{2}$
$v_{3}=11u_{1}-4u_{2}+u_{3}$
$v_{4}=-27u_{1}+11u_{2}-2u_{3}+u_{4}$
即:
$$[v]_{u}=\begin{bmatrix} 1 & -2 & 11 &-27 \\
0 & 1 & -4 & 11 \\
0 & 0 & 1 & -2 \\
0 & 0 & 0 & 1
\end{bmatrix}$$

也可得:
$u_{1} = v_{1}$
$u_{2}=2v_{1}+v_{2}$
$u_{3}=-3v_{1}+4v_{2}+v_{3}$
$u_{4}=-v_{1}-3v_{2}+2v_{3}+v_{4}$
即:
$$[u]_{v}=\begin{bmatrix} 1 & 2 &-3 &-1\\ 
0 & 1 & 4 & -3\\
0 & 0 & 1 & 2 \\
0 & 0 & 0 & 1
 \end{bmatrix}$$

给定向量$\omega=(6,-1,2,2)$，容易验证:
$\omega=v_{1}+3v_{2}+2v_{3}+v_{4}$
也就是:
$[w]_{v}=\begin{bmatrix} 1 \\ 3 \\ 2 \\ 1 \end{bmatrix}$
那么:
$$[w]_{u}=[v]_{u}[w]_{v}= \begin{bmatrix} 1 & -2 & 11 & -27 \\ 
0 & 1 & -4 & 11 \\
 0 & 0 & 1 & -2 \\
 0 & 0 & 0 & 1\end{bmatrix} \begin{bmatrix} 1 \\ 3\\ 2 \\ 1 \end{bmatrix} 
= \begin{bmatrix} -10 \\ 6 \\ 0 \\ 1\end{bmatrix}$$

容易验证:
(1) $w=-10u_{1}+6u_{2}+u_{4}$
(2) $[u]_{v}^{-1} = [v]_{u}$

---
## 仿射变换
线性变换无法表达一类重要的变换——平移变换。平移变换表达的是对于点$p=(x,y,z)$经过$d=(\alpha_{x}, \alpha_{y},\alpha_{z})$所表示的位移后得到点$p'=(x',y',z')$的过程，表示为：
$p'=p+d$
我们尝试寻找变换$T$满足:$T(x)=Ax=p'=\begin{bmatrix} x+\alpha_{x} \\ y+\alpha_{y} \\ z + \alpha_{z} \end{bmatrix}$
当$d \neq 0$时，上式中$T(0) \neq 0$，由式子1可知，这不是线性变换。因此需要引入仿射变换的概念。

仿射变换与线性变换不同之处在于，线性变换保持原点位置不变，而仿射变换可以改变原点的位置。仿射变换包括线性变换，例如旋转，缩放等变换，特殊地是仿射变换包括平移变换。例如$f(x)=2x$是一个线性变换，也是仿射变换；而$f(x)=2x+3$是仿射变换。感兴趣的可以参考[What is the difference between linear and affine function](http://math.stackexchange.com/questions/275310/what-is-the-difference-between-linear-and-affine-function)。一般而言，仿射变换是 线性变换+平移变换。

使用3x3矩阵无法表达平移变换(当$d \neq 0$时方程$T(x)=A_{3 \times 3}x=p'$无解，感兴趣可以参考[Reason for homogeneous](http://sunshine2k.blogspot.com/2011/12/reason-for-homogeneous-4d-coordinates.html))，但是以一个统一的方式表达变换，在计算中将便于计算，因此需要引入**齐次坐标系**的概念来完成这个目标。从编程角度来讲，在OpenGL中引入齐次坐标系主要是为了[表达平移变换和投影变换中的透视除法](http://computergraphics.stackexchange.com/questions/1536/why-are-homogeneous-coordinates-used-in-computer-graphics)。如果你要从数学角度理解齐次坐标系，这对数学要求较高，将会涉及到射影几何等概念，感兴趣的话可以参考[The Truth Behind Homogeneous Coordinates](http://deltaorange.com/2012/03/08/the-truth-behind-homogenous-coordinates/)。

### 齐次坐标(homogeneous coordinate)
齐次坐标是在原来坐标的基础上添加了一个$w$成分。在3D中使用$(x,y,z)$既能表示点，又能表示向量，容易引起混淆。使用齐次坐标能够克服这个困难。具体做法是，在由原点$P_{0}$和基$v_{1},v_{2},v_{3}$所定义的坐标系中，任意点$P$可以表示为:
$P=\alpha_{1}v_{1} + \alpha_{2}v_{2} + \alpha_{3}v_{3}+P_{0}$。
定义标量0和1与点的乘法为:
$0.P = 0, 1.P = P$，则可以把点P表示为:
$P=\begin{bmatrix} \alpha_{1} & \alpha_{2} & \alpha_{3} & 1 \end{bmatrix} \begin{bmatrix}v_{1} \\ v_{2} \\ v_{3} \\ P_{0} \end{bmatrix}$
可以把向量$w=\beta_{1}v_{1}+\beta_{2}v_{2}+\beta_{3}v_{3}$表示为:
$w=\begin{bmatrix} \beta_{1} & \beta_{2} & \beta_{3} & 0 \end{bmatrix} \begin{bmatrix}v_{1} \\ v_{2} \\ v_{3} \\ P_{0} \end{bmatrix}$。
因此，使用齐次坐标系表示点形式为:
$P=\begin{bmatrix} \alpha_{1}  \\  \alpha_{2} \\  \alpha_{3} \\ 1 \end{bmatrix}$
表示向量为:
$w=\begin{bmatrix} \beta_{1} \\ \beta_{2} \\ \beta_{3} \\ 0 \end{bmatrix}$

这里齐次坐标的$w$成分为0或者1，在后面投影变换的透视除法时还会看到$w$成分的其他值。

### 使用4x4矩阵表示仿射变换
对于上面的平移变换:
$T(x)=Ax=p'=\begin{bmatrix} x+\alpha_{x} \\ y+\alpha_{y} \\ z + \alpha_{z} \end{bmatrix}$
当$A$为4x4矩阵时，使用齐次坐标，x表示为$p=(x,y,z,1)$, $d=(\alpha_{x}, \alpha_{y},\alpha_{z},0)$, $p'=(x',y',z',1)$。
可以计算得到$A$为:
$$A= \begin{bmatrix} 1 & 0 & 0 & \alpha_{x} \\ 
0 & 1 & 0 & \alpha_{y} \\
0 & 0 & 1 & \alpha_{z} \\
0 & 0 & 0 & 1
 \end{bmatrix}$$.
 上面的矩阵A对应平移变换，对于线性变换，矩阵A中：
 $d=(\alpha_{x}, \alpha_{y},\alpha_{z},0)=(0,0,0)$。
 也就是说矩阵A的结构为:
![这里写图片描述](http://img.blog.csdn.net/20160518192008168)

### 使用4x4矩阵表示坐标转换
前面介绍的基变换与坐标变换，没有考虑到原点，两个坐标系的原点位置可能不同，因此需要使用4x4矩阵调整下上面的结果。对于参考点$P_{0}$和基$u=\{u_{1},u_{2},\cdots,u_{n}\}$，以及参考点$Q_{0}$和基$v=\{ v_{1},v_{2},\cdots, v_{n}\}$,除了上面的等式:
$u_{1}=\lambda_{11}v_{1} + \lambda_{12}v_{2}+\lambda_{13}v_{3}$
$u_{2}=\lambda_{21}v_{1} + \lambda_{22}v_{2}+\lambda_{23}v_{3}$
$u_{3}=\lambda_{31}v_{1} + \lambda_{32}v_{2}+\lambda_{33}v_{3}$
还需要添加一个$Q_{0}$与$P_{0}$关系式:
$Q_{0} = P_{0} + \lambda_{41}v_{1} + \lambda_{42}v_{2}+\lambda_{43}v_{3}$
写成矩阵形式为:
$\begin{bmatrix} u_{1} \\ u_{2} \\ u_{3} \\ Q_{0}\end{bmatrix}= M\begin{bmatrix} v_{1} \\ v_{2} \\ v_{3} \\ P_{0}\end{bmatrix}$

其中M是一个4x4矩阵:
$$M=\begin{bmatrix} \lambda_{11} & \lambda_{12} & \lambda_{13} & 0 \\ 
\lambda_{21} & \lambda_{22} & \lambda_{23}& 0 \\ 
\lambda_{31} & \lambda_{32} & \lambda_{33}& 0 \\ 
\lambda_{41} & \lambda_{42} & \lambda_{43}& 1 
\end{bmatrix}$$。
坐标转换的其余部分，与上面讲述的3x3矩阵类似。

####坐标转换示例1
下图所示的旋转坐标系后，求取原坐标系下点p在[s,t]坐标系下坐标，如下图所示：
![旋转](http://img.blog.csdn.net/20140917221645078)
直接利用极坐标推导的过程，请参考[ 坐标旋转变换公式的推导](http://blog.csdn.net/tangyongkang/article/details/5484636)。我们使用上面介绍的寻找基向量之间关系的方法来计算这个变换矩阵。
已知$xoy$坐标系下基$e=(e_{1}, e_{2})$用$sot$坐标系的基表示矩阵为:
$$[e]_{sot} = \begin{bmatrix} cos\theta & sin\theta \\
-sin\theta & cos\theta
\end{bmatrix}$$
那么:
$[w]_{sot} = [e]_{sot}[w]_{xoy}$
即:
$$\begin{bmatrix} s \\ t\end{bmatrix}= \begin{bmatrix} cos\theta & sin\theta \\
-sin\theta & cos\theta
\end{bmatrix} \begin{bmatrix} x \\ y \end{bmatrix}$$

#### 坐标转换示例2
这个例子来自参考资料[1]。
如下图所示，存在两个坐标系，世界坐标系和照相机坐标系：
![这里写图片描述](http://img.blog.csdn.net/20140918165850498)

其中照相机坐标系的原点位于世界坐标系的$P(1,0,1,1)$处，并且照相机指向世界坐标系的原点，则照相机指向向量$n=(-1,0,-1,0)^{T}$，照相机的观察正向同世界坐标系地y轴一样，为$v=(0,1,0,0)^{T}$,利用叉积$n \times v$计算出相机的第三个正交的方向$u=(1,0,-1,0)^{T}$。
那么点O和xyz轴构成的世界坐标系和以点P及uvn构成的照相机坐标系之间存在转换关系。其中uvnP在世界坐标系的表示为:
$$[uvnP]_{xyzO}=\begin{bmatrix} 1 & 0 & -1 & 1 \\
0 & 1 & 0 & 0 \\
-1 & 0 & -1 & 1 \\
0 & 0 & 0 & 1
\end{bmatrix}$$
xyzO在照相机坐标系下的表示为:
$$[xyzO]_{uvnP}=[uvnP]_{xyzO}^{-1}= \begin{bmatrix} 
1/2 & 0 & -1/2 & 0 \\
0 & 1 & 0 & 0 \\
-1/2 & 0 & -1/2 & 1 \\
0 & 0 & 0 & 1
\end{bmatrix}$$
则世界坐标系的原点O在uvnP坐标系下的坐标为:

$$[O]_{uvnP} = [xyzO]_{uvnP}[O]_{xyzO}=\begin{bmatrix} 1/2 & 0 & -1/2 & 0 \\
0 & 1 & 0 & 0 \\
-1/2 & 0 & -1/2 & 1 \\
0 & 0 & 0 & 1
\end{bmatrix} \begin{bmatrix} 0 \\ 0  \\ 0 \\ 1 \end{bmatrix} = \begin{bmatrix} 0 \\ 0 \\ 1 \\ 1\end{bmatrix}$$
这个即求出世界坐标系的原点在照相机坐标系下的$(0,0,1,1)$处。

同时照相机坐标系下的原点P在世界坐标系$xyzO$下的表示为:
$$[P]_{zyzO}=[uvnP]_{zyzO}[P]_{uvnP} = \begin{bmatrix} 
1 & 0 & -1 & 1 \\
0 & 1 & 0 & 0 \\
-1 & 0 & -1 & 1 \\
0 & 0 & 0 & 1
\end{bmatrix}  \begin{bmatrix} 0 \\ 0  \\ 0 \\ 1 \end{bmatrix} =  \begin{bmatrix} 1 \\ 0  \\ 1 \\ 1 \end{bmatrix}$$
这个即求出了照相机坐标系的原点在世界坐标系的$P(1,0,1,1)$处。
这个例子说明了不同坐标系下坐标之间的转换，后面还会详细介绍OpenGL中使用的坐标转换。

## 参考资料
[1]《交互式计算机图形学-基于OpenGL着色器的自动向下方法》电子工业出版社 Edward Angle等著
[2]《3D数学基础：图形与游戏开发》  清华大学出版社
[3]《线性代数》武汉大学数学与统计学院 高等教育出版社 齐民友主编 
[4] [Linear Transformations](http://linear.ups.edu/html/section-LT.html)
[5] [matrices_linear_transformations](http://mathinsight.org/matrices_linear_transformations)
[6] [Linear transformations](http://mathinsight.org/linear_transformation_definition_euclidean)
[7] [Objects in Motion](http://alfonse.bitbucket.org/oldtut/Positioning/Tutorial%2006.html)
[8] [很棒的 LINEAR TRANSFORMATIONS 课件](https://rutherglen.science.mq.edu.au/wchen/lnlafolder/la08.pdf)