写在前面
  上一节[坐标变换过程](http://blog.csdn.net/wangdingqiaoit/article/details/51594408)，总结了OpenGL中顶点的处理过程。有了前面的基础，从本节开始，我们可以开始学习OpenGL中包括光照、模型加载等主题。光照是一个复杂的主题，本节学习简单的Phong reflection model.本节示例程序均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/lighting)。

> **本节内容整理自:**
> 1. learnopengl.com [Basic Lighting](http://learnopengl.com/#!Lighting/Basic-Lighting)
> 2.[Modern OpenGL 06 – Diffuse Point Lighting](http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/)

通过本节可以了解到

- 颜色与光照的关系
- 简单实现的Phong Reflection Model
- Gouraud shading和Phong Shading的对比


## 颜色与光照的关系
我们看到的物体的颜色，实际上是光照射物体后反射的光进入眼睛后感受到的颜色，而不是物体实际材料的颜色。太阳的白光包含了所有我们可以感知的颜色，这个可以将白光通过棱镜折射后，将白色光分离为各种颜色的光。将一束白光照射到红颜色的车身上，光经过车身，一部分被吸收，一部分被反射进入人的眼睛，我们感知到的颜色就是这个反射后进入眼睛的光的颜色，如下图所示(来自:[Modern OpenGL 06 – Diffuse Point Lighting](http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/))：

![光的反射1](http://img.blog.csdn.net/20160611212429716)
图中，红色的表面吸收了蓝色和绿色成分，将红色反射出来。颜色吸收和反射的过程可以表示为：
$LightIntensity * ObjectColor = Relectcolor$
计算为:
$(R,G,B)*(X,Y,Z)=(XR,YG,ZB)$
则上面的过程表示为:
$(1,1,1)*(1,0,0) = (1,0,0)$
如果将cyan (blue + green) 颜色光束照射到车身，车身会是什么颜色呢？会是黑色的，因为$(0,1,1)*(1,0,0)=(0,0,0)$,这个过程如下图所示(来自:[Modern OpenGL 06 – Diffuse Point Lighting](http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/))：

![光的反射2](http://img.blog.csdn.net/20160611213122289)

如果将cyan (blue + green) 颜色照射到magenta (red + blue) 颜色的表面，那么结果会是什么颜色呢？同理，我们可以得到结果颜色为蓝色，如下图所示来自:[Modern OpenGL 06 – Diffuse Point Lighting](http://www.tomdalling.com/blog/modern-opengl/06-diffuse-point-lighting/))：

![光的反射3](http://img.blog.csdn.net/20160611213343136)
在实际场景中，光的强度的各个分量可以在[0,1]之间变化，材料表面的颜色分量也可以在[0,1]之间变化，例如光照射到一个玩具表面的计算过程为：
```C++
glm::vec3 lightColor(0.33f, 0.42f, 0.18f);
glm::vec3 toyColor(1.0f, 0.5f, 0.31f);
glm::vec3 result = lightColor * toyColor; // = (0.33f, 0.21f, 0.06f);
```

## Phong Reflection Model
要模拟现实的光照是困难的，例如实际光照中，一束光可以经过场景中若干物体反射后，照射到目标物体上，也可以是直接照射到目标物体上。其中经过其他物体反射后再次照射到目标物体上，这是一个递归的过程，将会无比复杂。因此实际模拟光照过程中，总是采用近似模型去接近现实光照。Phong Reflection Model是经典的光照模型，它计算光照包括三个部分：环境光+漫反射光+镜面光，一共三个成分，如下图所示([来自wiki ,作者Brad Smith](http://en.wikipedia.org/wiki/File:Phong_components_version_4.png))：
![Phong Reflection Model](http://img.blog.csdn.net/20160611214522357)

### 环境光成分
环境光是场景中光源给定或者全局给定的一个光照常量，它一般很小，主要是为了模拟即使场景中没有光照时，也不是全部黑屏的效果。场景中总有一点环境光，不至于使场景全部黑暗，例如远处的月亮，远处的光源。
环境光的实现为：
```C++
// 环境光成分
float	ambientStrength = 0.1f;
vec3	ambient = ambientStrength * lightColor * objectColor;
```
给定环境光后，场景效果如下图所示：
![环境光](http://img.blog.csdn.net/20160611215104099)
这里使用了两个着色器绘图。一个着色器用来绘制光源，光源用一个缩小的立方体来模拟，如图中白色立方体所示；另一个着色器用来绘制我们的物体，这里只显示了一个大的立方体。当场景中只有环境光时，立方体只能很暗的显示。

### 漫反射光成分
漫反射光成分，是光照中的一个主要成分。漫反射光强度与光线入射方向和物体表面的法向量之间的夹角$\theta$相关。当$\theta$ = 0时，物体表面正好垂直于光线方向，这是获得的光照强度最大；当$\theta$ = 90时物体表面与光线方向平行，此时光线照射不到物体，光的强度最弱；当$\theta \gt 90$后，物体的表面转向到光线的背面，此时物体对应的表面接受不到光照。入射角度如下图所示：
![AOI](http://img.blog.csdn.net/20160611220411265)

这里需要的向量包括:
1.光源和顶点位置之间的向量L 需要计算。
2.法向量N  通过顶点属性里指定 经过模型和视变换后需要重新计算。

**有两种方法执行向量L和N的计算。一种方式是在世界坐标系中计算，另一种是在相机坐标系中计算，两种方法都可以实现。**

这里以在世界坐标系中计算L和N为例进行说明，在相机坐标系中计算也有类似操作。在世界坐标系中，计算L时，光源lightPos是在世界坐标系中指定的位置，直接使用即可。顶点位置需要变换到世界坐标系中，利用Model矩阵即可，使用式子:
$FragPos = vec3(model * vec4(position, 1.0));\tag{变换后顶点位置}$
在计算N时需要注意，我们不能直接利用$Model*normal$来获取变换后的法向量，应该使用式子:
$Normal = mat3(transpose(inverse(model)))*normal \tag{变换后法向量}$。
这个式子的具体推导过程，可以参考[The Normal Matrix](http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/)。
综上所述，顶点着色器中计算顶点位置和法向量代码为：
```C++
#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textCoord;
layout(location = 2) in vec3 normal;

out vec3 FragPos;
out vec2 TextCoord;
out vec3 FragNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
gl_Position = projection * view * model * vec4(position, 1.0);
FragPos = vec3(model * vec4(position, 1.0)); // 在世界坐标系中指定片元位置
TextCoord = textCoord;
mat3 normalMatrix = mat3(transpose(inverse(model)));
FragNormal = normalMatrix * normal; // 计算法向量经过模型变换后值
}
```
在片元着色器中，计算漫反射光成分的代码为：
```C++
// 漫反射光成分 此时需要光线方向为指向光源
vec3	lightDir = normalize(lightPos - FragPos);
vec3	normal = normalize(FragNormal);
float	diffFactor = max(dot(lightDir, normal), 0.0);
vec3	diffuse = diffFactor * lightColor * objectColor;
```
这里使用max(dot(lightDir, normal), 0.0)主要是为了防止当光线和法向量夹角大于90后，取值为负的情况，因此使用max保证漫反射光照系数在[0.0,1.0]范围内。
添加了漫反射光成分后的效果如下图所示：
![漫反射光](http://img.blog.csdn.net/20160612224732451)

## 镜面反射光成分
镜面光成分模拟的是物体表面光滑时反射的高亮的光，镜面光反映的通常是光的颜色，而不是物体的颜色。计算镜面光成分时，要考虑光源和顶点位置之间向量L、法向量N、反射方向R、观察者和顶点位置之间的向量V之间的关系，如下图所示(来自:[Lighting and Material](http://math.hws.edu/graphicsbook/c7/s2.html))：
![light equatation](http://img.blog.csdn.net/20160611220215053)
当R和V的夹角$\theta$越小时，人眼观察到的镜面光成分越明显。镜面反射系数定义为:
$specFactor = cos(\theta)^{s}$
其中$s$表示为镜面高光系数（shininess ），它的值一般取为2的整数幂，值越大则高光部分越集中，例如下面图中，测试了几种不同的高光系数，效果如下所示：
![镜面高光系数](http://img.blog.csdn.net/20160612224338214)

计算镜面光成分过程为：
```C++
// 镜面反射成分 此时需要光线方向为由光源指出
float	specularStrength = 0.5f;
vec3	reflectDir = normalize(reflect(-lightDir, normal));
vec3	viewDir = normalize(viewPos - FragPos);
float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 32); // 32为镜面高光系数
vec3	specular = specularStrength * specFactor * lightColor * objectColor;
```
这里需要注意的是，利用reflect函数计算光的出射方向时，要求入射方向指向物体表面位置，因此这里翻转了lightDir，计算为：
```C++
   vec3	reflectDir = normalize(reflect(-lightDir, normal));
```
将上述三种光成分叠加后，成为最终物体的颜色，片元着色器中实现为：
```C++
   vec3	result = ambient + diffuse + specular 
   color = vec4(result , 1.0f);
```
绘制效果如下图所示：
![包含三个成分后](http://img.blog.csdn.net/20160612224938157)

## per-vertex 和per-fragment实现光照的对比
上面我们实现的光照计算是在片元着色器中进行的，这种是基于片元计算的，称之为[Phong shading](http://en.wikipedia.org/wiki/Phong_shading)。在过去OpenGL编程中实现的是在顶点着色器中进行光照计算,这是基元顶点的计算的，称之为[Gouraud Shading](http://en.wikipedia.org/wiki/Gouraud_shading)。Gouraud Shading和Phong shading，两者的效果对比如下图所示(来自learnopengl.com [Basic Lighting](http://learnopengl.com/#!Lighting/Basic-Lighting))：
![per-vertex](http://img.blog.csdn.net/20160612230600288)

基元顶点计算光照的优势在于顶点数目比片元数目少，因此计算速度快，但是基于顶点计算的光照没有基元片元的真实，主要是基元顶点计算时，只计算了顶点的光照，而其余片元的光照由插值计算得到，这种插值后的光照显得不是很真实，需要使用更多的顶点来加以完善。例如下面的图中，分别显示了使用少量和大量顶点的基于顶点的光照计算效果:

![low](http://img.blog.csdn.net/20160612231520570)
![high](http://img.blog.csdn.net/20160612231540317)

使用基于片元的光照计算时能够获取更为平滑的光照效果。实现基元顶点的光照计算过程，即将上述在片元着色器中的光照计算过程迁移到顶点着色器中执行。

## 最后的说明
在计算光照的过程中，注意使用的向量一定要单位化，因为$cos\theta$值的计算依赖于两个参与点积的向量是单位向量这一事实，否则计算会出错。另外在世界坐标系还是在相机坐标系中进行光照计算都是可以的，这个取决于你的喜好，但是要注意将顶点位置、法向量都变换到同一个坐标系下进行光照计算。

本节实现的Phong reflection model还不够完善，一方面从光源角度看，属于点光源，但是缺少随着距离的衰减；另一方面从物体的材质角度看，没有反映出物体不同部分对光感受的强度不同这一特点，需要使用材质属性加以改进。这些内容将放在下一节中进行学习。