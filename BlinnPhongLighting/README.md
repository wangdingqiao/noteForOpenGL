写在前面
    在前面[基础光照部分](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)，我们学习了Phong Shading模型，Blinn-Phong模型对Phong模型的镜面光成分进行了改进，虽然在物理上解释没有Phong好，但是能更好地模拟光照。本节代码可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/BlinnPhongLighting)。

> 本节内容整理自：
> 1.[www.learnopengl.com](http://www.learnopengl.com/#!Advanced-Lighting/Advanced-Lighting)
> 2.[Blinn-Phong Model](http://alfonse.bitbucket.org/oldtut/Illumination/Tut11%20BlinnPhong%20Model.html)


# Phong不能处理的情况

我们知道，Phong模型在计算镜面光系数为：
```cpp
float   specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 32); // 32为镜面高光系数
```
这里的计算由反射向量和观察向量决定，当两者的夹角$\theta$超过90时，截断为0.0，则没有了镜面光成分。因此Phong模型能处理的是下面的左图中($\theta \le 90 $)的情况，而对于右图中($\theta \gt 90 $)的情况则镜面光成分计算为0(来自[Advanced-Lighting](http://www.learnopengl.com/#!Advanced-Lighting/Advanced-Lighting))。
![这里写图片描述](http://img.blog.csdn.net/20161029152218930)

而右图的这种情况实际上是存在的，将镜面光成分取为0，没有很好地体现实际光照情况。例如下面的图表示的是，镜面光系数为1.0，法向量为(0.0,1.0,0.0)的平面位置在-0.5,光源在原点时，观察者在(0,0,4.0)位置时，光照展示的情形：

![phong模型的边缘问题](http://img.blog.csdn.net/20161029152430993)

这里我们看到，Phong的镜面光成分，在边缘时立马变暗，这种对比太明显，不符合实际情形。

----

**为什么会产生这样一个光线明暗分明的情形?** 我尝试这样推导，对此不感兴趣地可以跳过。

首先记表面位置为$fragPos=(x, -0.5, z)$, 光源位置为$lightPos=(0.0, 0.0, 0.0)$，则光照向量为：

$$\begin{align} L &= -(lightPos - fragPos) \\
&= (x, -0.5, z)\tag{light direction}\end{align}$$
法向量为：
$$N = (0.0, 1.0, 0.0)\tag{surface normal}$$
根据[reflect函数的计算原理](https://www.opengl.org/sdk/docs/man4/html/reflect.xhtml)，得到反射向量为：
$$\begin{align} R &= L - 2.0 * dot(N, L) * N \\
&= (x, -0.5, z) - 2.0 * (-0.5)*(0.0, 1.0,0.0) \\
&= (x, 0.5,z)\tag{surface reflection}\end{align}$$

设观察点位置为$(x',y',z')$，则观察向量为：
$$\begin{align} V &=(x',y',z') - fragPos \\ 
&= (x'-x, y'+0.5,z'-z) \tag{viewer direction}\end{align}$$

那么反射向量和观察向量的点积为：

$$\begin{align}dot(R,V) &= (xx'-x^2+0.5y'+0.25+zz'-z^2) \\
&=-[(x-\frac{x'}{2})^2+(z-\frac{z'}{2})^2-0.25(x'^2+z'^2+1+2y')]  \end{align}$$
记$\delta = 0.25(x'^2+z'^2+1+2y')$

令$0 \leq dot(R,V) \leq 1$,得到：

$ \delta-1 \leq (x-\frac{x'}{2})^2+(z-\frac{z'}{2})^2 \leq \delta$
由此可以看出，位置在平面y=-0.5上的点，以适当位置观察时，会形成两个同心圆，在两个同心圆之间的部分则满足$0 \leq dot(R,V) \leq 1$，这部分有镜面光，其余部分截断为0.0，立马变暗，因此有这种明暗对比。
也就是说当观察向量和反射向量超过90度，这种截断引起了明显的明暗对比，这种情形在Blinn-Phong中得到改善。

--------
# Blinn-Phong
Blinn-Phong模型镜面光的计算，采用了半角向量(half-angle vector)，这个向量是光照向量L和观察向量V的取中向量，如下图所示(来自[Blinn-Phong Model](http://alfonse.bitbucket.org/oldtut/Illumination/Tut11%20BlinnPhong%20Model.html))：

![半角向量](http://img.blog.csdn.net/20161029153904681)

计算为: $H = \frac{L+V}{||L+V||}$

当观察向量与反射向量越接近，那么半角向量与法向量N越接近，观察者看到的镜面光成分越强。

对比Phong和Blinn-Phong计算镜面光系数为：

```cpp
 vec3	viewDir = normalize(viewPos - fs_in.FragPos);
float	specFactor = 0.0;
if(blinn)  // 使用Blinn-Phong specular 模型
{
vec3 halfDir = normalize(lightDir + viewDir);
specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); 
}
else	// 使用Phong specular模型
{
vec3	reflectDir = normalize(reflect(-lightDir, normal)); // 此时需要光线方向为由光源指出
specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 8.0); 
}
```

使用半角向量后，保证了半角向量H与法向量N的夹角在90度范围内，能够处理上面对比图中右图所示的情形。下面是镜面高光系数为0.5时使用Blinn-Phong渲染效果：

![Blinn-Phong](http://img.blog.csdn.net/20161029154828599)

下图是镜面高光系数为0.5时使用Phong渲染效果:

![Phong](http://img.blog.csdn.net/20161029154951305)

一般地，使用Blinn-Phong模型时要得到相同强度的镜面光，镜面系数需要为Phong模型的2-4倍，例如Phong模型的镜面高光系数设置为0.8，可以设置Blinn-Phong模型的系数为32.0。

关于Phong和Blinn-Phong模型更多地对比，可以参考[Relationship between Phong and Blinn lighting model](https://seblagarde.wordpress.com/2012/03/29/relationship-between-phong-and-blinn-lighting-model/)。

