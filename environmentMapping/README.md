写在前面
    [上一节](http://blog.csdn.net/wangdingqiaoit/article/details/52506893)初步学习了使用cubeMap创建天空包围盒，本节继续深入Cubemap这个主题，学习环境纹理贴图。本节示例程序均可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/environmentMapping)。

> 本节内容整理自:
> 1.[www.learnopengl.com cubemaps](http://www.learnopengl.com/#!Advanced-OpenGL/Cubemaps)

# 环境纹理贴图

同上一节的Cubemap创建天空包围盒有些类似，创建环境纹理贴图也是对当前待渲染物体，从包围的环境纹理上采样作为这个物体的纹理而渲染出的逼真效果。本节介绍环境纹理贴图主要的方式包括:reflection(反射贴图)和refraction(折射贴图)。

# Reflection 反射贴图

在[上一节cubemaps]((http://blog.csdn.net/wangdingqiaoit/article/details/52506893))中，我们提到对立方体纹理进行采样，需要使用3维向量(s,t,r)，而当立方体中心处于原点时，立方体的顶点位置就可以作为这个采样的坐标。对于反射贴图，我们也同样需要一个纹理坐标，不过这个向量的计算过程如下图所示([来自 www.learnopengl.com](http://www.learnopengl.com/#!Advanced-OpenGL/Cubemaps))：

![reflection](http://img.blog.csdn.net/20160916204421243)

图中向量$I$表示观察向量，注意它从观察者位置指出，$N$表示顶点对应的法向量，而计算出来的反射向量$R$则作为从Cubemap采样的向量。在[光照基础](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)，一节我们已经见过使用reflect函数计算反射向量了，这里再次说明下。我们计算向量的过程都可以在世界坐标系或者相机坐标系，只要统一一个坐标系即可。这里我们使用世界坐标系，在顶点着色器中，实现为：

```cpp
   #version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 FragNormal;
out vec3 FragPos;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	FragPos = vec3(model * vec4(position, 1.0));	// 在世界坐标系中指定
	mat3 normalMatrix = mat3(transpose(inverse(model)));
	FragNormal = normalMatrix * normal;	// 计算法向量经过模型变换后值
}
```
上面在世界坐标系中计算转换后的法向量时使用了公式：

$FragPos = vec3(model * vec4(position, 1.0));\tag{变换后顶点位置}$
$FragNormal = mat3(transpose(inverse(model)))*normal \tag{变换后法向量}$
对于公式不熟悉的，可以回过头去参考[光照基础](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)一节。


在片元着色器中实现为:

```cpp
   #version 330 core

in vec3 FragNormal;
in vec3 FragPos;

uniform samplerCube envText; // 环境纹理
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	vec3 viewDir = normalize(FragPos - cameraPos); // 注意这里向量从观察者位置指出
	vec3 reflectDir = reflect(viewDir, normalize(FragNormal));
	color = texture(envText, reflectDir);	// 使用反射向量采样环境纹理
}
```

注意上面计算过程中向量的方向和单位化。这里输入的环境纹理依然是我们的天空包围盒cubemap纹理。绘制上一节的立方体，使用反射贴图得到的效果如下图所示：

![这里写图片描述](http://img.blog.csdn.net/20160916205442505)

这个贴图得到的效果是，立方体的表面反射包围盒的纹理。
如果使用球体模型的话，则能得到更逼真的效果：

![球体反射贴图](http://img.blog.csdn.net/20160916205649383)

这个球体模型，可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/resources/models/sphere)。


# Refraction 折射贴图
与反射相对应，当光从一种材质进入另一种材质时将会发生折射，满足折射定律。这里我们采用折射后得到的向量，作为采样纹理的向量，计算如下图所示([来自 www.learnopengl.com](http://www.learnopengl.com/#!Advanced-OpenGL/Cubemaps))：

![refraction](http://img.blog.csdn.net/20160916210203532)

这里的$I$为入射向量，从观察者位置指出，$N$仍然是法向量，而得到的折射向量$R$作为采样纹理的向量。可以看出当光从空气进入水中时，发生了折射现象，折射向量$R$与原始的入射光线$I$发生了偏离。
在计算折射向量时，需要使用到折射率，这个参数，给出几种材料的折射率参考数据，如下表：

| 材料| 折射率|
|--------|-----|
|空气	|1.00|
|水| 	1.33|
|冰 	|1.309|
|玻璃| 	1.52|
|钻石| 	2.42|

从一种材质进入另一种材质，实际计算时使用两种材质的折射率的比例。实现refraction效果是，顶点着色器部分与上面相同，片元着色器需要修改，计算折射向量，如下：

```cpp
   #version 330 core

in vec3 FragNormal;
in vec3 FragPos;

uniform samplerCube envText; // 环境纹理
uniform vec3 cameraPos;

out vec4 color;

void main()
{
	float indexRation = 1.00 / 1.52;
	vec3 viewDir = normalize(FragPos - cameraPos); // 注意这里向量从观察者位置指出
	vec3 refractDir = refract(viewDir, normalize(FragNormal), indexRation);
	color = texture(envText, refractDir);	// 使用 折射向量 采样环境纹理
}
```
上面着色器中，refractive的第三个参数是折射率的比例，这里我们模拟的是从空气进入玻璃。绘制上面的立方体，得到的效果像是透明玻璃：

![refraction](http://img.blog.csdn.net/20160916210903190)


# 环境纹理贴图和模型加载

在前面[AssImp纹理加载](http://blog.csdn.net/wangdingqiaoit/article/details/52014321)一节，我们使用AssImp库加载了一个纳米战斗服模型，这里对模型使用反射环境贴图加以改进。之前使用的是diffuse map 和specular map,这里将使用的方法称之为reflection map。通过加载模型中的reflection map，决定渲染的物体中哪部分需要做反射环境贴图以及环境贴图的强度系数，而不是像上面球体那样，整个执行反射环境贴图。

利用AssImp加载Reflection map，遇到的麻烦在于，AssImp对Reflection map默认支持不好，因此这里使用的技巧是修改obj资源和代码，做出调整。你可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/resources/models/nanosuit_reflection)，修改后的模型。

在资源中使用aiTextureType_AMBIENT作为reflection map，在model.h代码中添加处理:

```cpp
// 获取Reflection 注意: AssImp对Reflection支持不好 所以这里采用ambient_map
// 除了这里的代码 还需要修改对应的obj文件
std::vector<Texture> reflectionTexture;
this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_AMBIENT, reflectionTexture);
textures.insert(textures.end(), reflectionTexture.begin(), reflectionTexture.end());
```
同时在mesh.h中也要添加相应的处理，在片元着色器中修改为：

```cpp
#version 330 core

in vec3 FragNormal;
in vec3 FragPos;
in vec2 TextCoord;

uniform samplerCube envText;	// 环境纹理
uniform sampler2D texture_diffuse0;
uniform sampler2D specular_diffuse0;
uniform sampler2D texture_reflection0; // 反射map
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	vec4	diffuseColor = texture(texture_diffuse0, TextCoord);
	float	relefctIntensity = texture(texture_reflection0, TextCoord).r; 
	vec4	reflectColor = vec4(0.0, 0.0, 0.0, 0.0);
	if(relefctIntensity > 0.1) // 决定是否开启
	{
		vec3 viewDir = normalize(FragPos - cameraPos); 
		vec3 reflectDir = reflect(viewDir, normalize(FragNormal));
		reflectColor = texture(envText, reflectDir) * relefctIntensity;	// 使用反射向量采样环境纹理 使用强度系数控制
	}
	color = diffuseColor + reflectColor;
}
```
这里通过读取reflection map，采样纹理后，获取一个强度系数，根据强度系数来决定是否开启refleciton map，如果开启则输出颜色为diffuse map和reflection map的混合结果。

渲染时只输出diffuse map或者reflection map，以及最终得到的效果，对比如下图所示：

![reflection map](http://img.blog.csdn.net/20160916212116711)

对头部进行放大，我们看到了反射贴图的效果：

![reflection part](http://img.blog.csdn.net/20160916212243312)

反射贴图为模型的表面增加了环境成分，显得更加逼真，而且模型在移动过程中，贴图能够动态变换，反映位置的改变。


# 最后的说明
在实现本节内容过程中，需要注意，使用反射或者折射时需要提供物体表面法向量，不仅需要修改顶点属性数据，还要修改对应的顶点着色器，如果修改错误，可能产生错误效果如下图：

![这里写图片描述](http://img.blog.csdn.net/20160916212650396)

从图中结果，可以看到渲染基本的图形出错，可以立马联想到顶点属性数据配置出错。

同时在加载模型，使用reflection map时，天空包围盒使用的纹理单元，要更新为3， 因为前面加载了其他的specular map、diffuse map、reflection map:

```cpp
glActiveTexture(GL_TEXTURE3);
glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextId());
glUniform1i(glGetUniformLocation(shader.programId, "envText"), 3); 
```
如果没有更新这个配置，导致天空包围盒的纹理单元被其他纹理占用，将发生错误，例如可能的错误结果如下：

![refletion map error](http://img.blog.csdn.net/20160916213037525)

关于环境纹理贴图，还有一项技术称为dynamic environment mapping。通过借助framebuffer,为每个物体渲染6个包含场景中其他物体的纹理，从而构建一个环境纹理，然后实行贴图。这种动态贴图方式，由于在framebuffer中要为每个物体执行6次场景渲染，在保持较好性能开销下使用它需要很多技巧，没有这里介绍的天空包围盒这么容易使用。在后面时间充足时，可以学习下这个技术。

