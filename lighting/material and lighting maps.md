写在前面
这一节隔了好久了，今天终于有时间写完。[上一节光照基础](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)学习实现了Phong reflection model，但是还存在不足，本节使用材质属性，以及使用diffuse map和specular map改善上一节的实现。本节示例程序均可以在[我的github](https://github.com/wangdingqiao/noteForOpenGL/tree/master/lighting)下载。

> 本节内容整理自:
1.[www.learnopengl.com Materials](http://www.learnopengl.com/#!Lighting/Materials)
2.[www.learnopengl.com Lighting maps](http://www.learnopengl.com/#!Lighting/Lighting-maps)

通过本节可以了解到

- 使用材质属性使不同物体对光有不同反映

- 对光源的不同部分使用不同的强度

- 使用diffuse map和specular map使物体不同部分对光有不同反映


## 材质属性-不同物体对光有不同反映
现实世界中，不同的物体对光有不同的反映，例如钢做成的物体通常比土制的花瓶看起来更亮，木质的容器和钢做成的容器对光的反应也不一样。对于镜面反射光，不同的物体接受光照后，高光部分的半径大小也不一样。要模拟不同的物体接受光照后的效果，就需要考虑物体的材质属性，利用材质属性模拟出不同的效果。
上一节我们使用了环境光(ambient)，漫反射光(diffuse)，镜面反射光(specular)三种成分实现的Phong reflection mode。为物体指定材质属性时，也可以为物体指定这三个不同成分的光的强度作为材质属性，同时加上高光系数shininess，整个材质在**片元着色器**中定义为如下所示的结构体：

```C++
// 材质属性结构体
struct MaterialAttr
{
	vec3 ambient;	// 环境光
	vec3 diffuse;	 // 漫反射光
	vec3 specular;   // 镜面光
	float shininess; //镜面高光系数
};
```
利用上述4个属性来定义物体的材质，我们可以模拟出很多现实世界的物体，如何配出具有真实效果的物体光照效果需要丰富的经验，同时也要使用更复杂的模型，后面我们会学习如何加载模型。通过加载丰富的模型，配上光照后，将会更接近现实。

利用上面的材质，我们实现Phone reflection model的**片元着色器**代码变为：

```C++
void main()
{    
	// 环境光成分
	vec3 ambient = lightColor * material.ambient;
	 
	// 漫反射光成分
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightColor * (diff * material.diffuse);
	    
	 // 镜面光成分
	 vec3 viewDir = normalize(viewPos - FragPos);
	 vec3 reflectDir = reflect(-lightDir, norm);  
	 float spec = pow(max(dot(viewDir, 
		reflectDir), 0.0), material.shininess);
	 vec3 specular = lightColor * (spec * material.specular);  
	 vec3 result = ambient + diffuse + specular;
	 color = vec4(result, 1.0f);
}
```
在主程序中设置材质结构体的每个属性，使用结构体变量的分量来设置。例如定义材质结构体变量为：
```C++
uniform MaterialAttr material;
```
则使用分量来设置属性各个部分的实现为：
```C++
	   // 设置材料光照属性
		GLint objectAmbientLoc = glGetUniformLocation(shader.programId, "material.ambient");
		GLint objectDiffuseLoc = glGetUniformLocation(shader.programId, "material.diffuse");
		GLint objectSpecularLoc = glGetUniformLocation(shader.programId, "material.specular");
		GLint objectShininessLoc = glGetUniformLocation(shader.programId, "material.shininess");
		glUniform3f(objectAmbientLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(objectDiffuseLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(objectSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform1f(objectShininessLoc, 32.0f);
```
将lightColor设置为(1.0,1.0,1.0)，则执行程序，我们得到的效果如下：

![没有考虑光各个成分强度](http://img.blog.csdn.net/20160703230437261)

可以看到上面图中的物体太亮了，主要原因是这里没有像上一节一样使用ambientStrength,specularStrength等参数调节光源不同成分的强度，导致各个成分叠加到一起后，强度太大，因此十分明亮。要为光源不同成分指定不同的强度，这个类似于为物体指定材质的做法，我们定义光源结构体为：

```c++
// 光源属性结构体
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
```

在程序中设置光源属性，同理可以通过结构体变量的分量来设置，如下：
```C++
  GLint lightAmbientLoc = glGetUniformLocation(shader.programId, "light.ambient");
		GLint lightDiffuseLoc = glGetUniformLocation(shader.programId, "light.diffuse");
		GLint lightSpecularLoc = glGetUniformLocation(shader.programId, "light.specular");
		GLint lightPosLoc = glGetUniformLocation(shader.programId, "light.position");
		glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);
		glUniform3f(lightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lampPos.x, lampPos.y, lampPos.z);
```
其中lampPos是事先指定的光源位置，例如定义为：
```C++
   glm::vec3 lampPos(0.8f, 0.8f, 0.5f);
```
更改后的效果为：
![使用材质后的光照](http://img.blog.csdn.net/20160703231018325)

这个效果同上一节的没有什么大的区别，但是我们为光源的部分成分指定了不同的强度，这代替了上一节在着色器中指定的ambientStrength,specularStrength等参数；同时为物体使用了材质属性，更加灵活地控制物体接受光照时的效果。

如果控制光源的属性随着时间改变，我们可以得到好玩的效果如下图所示：
![变化的光源属性](http://img.blog.csdn.net/20160703234842784)

变换光源可以使用glfwGetTime函数来实现如下：

```C++
// 随时间变化的光源属性
glm::vec3 lightColor;
lightColor.x = sin(glfwGetTime() * 2.0f);
lightColor.y = sin(glfwGetTime() * 0.7f);
lightColor.z = sin(glfwGetTime() * 1.3f);
glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // 适当减小影响
glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
glUniform3f(lightAmbientLoc, ambientColor.x, ambientColor.y, ambientColor.z);
glUniform3f(lightDiffuseLoc, diffuseColor.x, diffuseColor.y, diffuseColor.z);
glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
```

## light maps-物体的不同部分对光有不同反映

上面提到了不同的物体对光有不同的反映，实际上同一物体的部分往往不是同一种材料构成的，例如汽车的车身喷漆了往往很光亮，而轮胎的橡胶部分则比较暗淡。为了更好的模拟现实中物体接受光照效果，我们应该为物体的不同部分指定不同的材质属性，而不是整个物体共用一个材质属性。

不同的部分对应不同的材质属性，这个有点类似于根据纹理坐标获取不同的纹素，我们这里的做法同纹理坐标类似，不过这里的做法取名为light maps。也就是为物体的不同部分指定不同的坐标，根据这个坐标获取从light map获取不同的材质属性，对应漫反射有diffuse map，镜面反射有specular map。当然还包括其他类型的light map，这里只学习diffuse map和specular map。

### 使用diffuse map
要使用diffuse map，同纹理坐标一样，我们需要在顶点属性中添加纹理坐标，定义顶点属性如下：
```C++
   // 指定顶点属性数据 顶点位置 纹理 法向量
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,1.0f,	// A
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,	// B
		0.5f, 0.5f, 0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,	// C
		0.5f, 0.5f, 0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,	// C
		-0.5f, 0.5f, 0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,	// D
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,	// A
		...省略
	};
```

同时要加载纹理作为diffuse map，如下：
```C++
   GLint diffuseMap = TextureHelper::load2DTexture("../../resources/textures/container_diffuse.png");
	shader.use();
	glUniform1i(glGetUniformLocation(shader.programId, "material.diffuseMap"), 0);
```
这里使用了封装的加载和建立纹理对象的方法 TextureHelper::load2DTexture，关于如何使用纹理，在前面[二维纹理映射](http://blog.csdn.net/wangdingqiaoit/article/details/51457675)一节已经介绍过了，如果不清楚，可以回过头去查看。

在着色器中，重新定义材质属性为：
```
   // 材质属性结构体
struct MaterialAttr
{
	sampler2D diffuseMap;// 根据位置取不同的材质属性
	sampler2D specularMap;
	float shininess; //镜面高光系数
};
```
物体材质部分，环境光一般和漫反射光相同，只是强度不同，因此计算环境光和漫反射光都使用diffuse map，计算如下：
```
   // 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, TextCoord));

	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(material.diffuseMap, TextCoord));
```
在上面的代码中，我们通过在材质属性中定义纹理对象sampler2D，然后使用纹理对象material.diffuseMap来计算环境光和漫反射光成分。如果对镜面反射光不使用specular map，我们得到的效果如下：

![只使用diffuse map](http://img.blog.csdn.net/20160703232940099)

上面的图中，使用的diffuse map是一个木制的容器，在图中我们看到高光部分很明显，但是对于木制容器，对于镜面光的反映应该不会这么大，因此我们通过specular map需要调整图中镜面光部分。

## 使用specular map
假若我们想要一个由钢做成的边架的木制容器，当接收镜面光时，肯定是钢架子很亮，而木制部分的镜面光比较弱，要达到这样一种效果，我们可以为钢架子部分指定较强的强度，而木制部分强度较弱。如果通过手动来设置肯定很麻烦了，同diffuse map一样，我们也是用类似的技术处理，这就是specular map。

一般而言镜面光的light map中，表示强度大可以用接近白色的颜色表示，强度弱则使用接近黑色的颜色表示。最终我们的specular map用下图来表示(来自[www.learnopengl.com light maps](http://www.learnopengl.com/img/textures/container2_specular.png))：

![specular map](http://img.blog.csdn.net/20160703233629767)

在主程序中加载diffuse map和specular map，如下:

```C++
   // Section3 准备diffuseMap和specularMap
	GLint diffuseMap = TextureHelper::load2DTexture("../../resources/textures/container_diffuse.png");
	GLint specularMap = TextureHelper::load2DTexture("../../resources/textures/container_specular.png");
	shader.use();
	glUniform1i(glGetUniformLocation(shader.programId, "material.diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(shader.programId, "material.specularMap"), 1);
```
在着色器中使用diffuse map和specular map完成光照计算过程为：
```C++
   void main()
{   
	// 环境光成分
	vec3	ambient = light.ambient * vec3(texture(material.diffuseMap, TextCoord));
	// 漫反射光成分 此时需要光线方向为指向光源
	vec3	lightDir = normalize(light.position - FragPos);
	vec3	normal = normalize(FragNormal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(material.diffuseMap, TextCoord));
	// 镜面反射成分 此时需要光线方向为由光源指出
	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - FragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3	specular = specFactor * light.specular * vec3(texture(material.specularMap, TextCoord));
	vec3	result = ambient + diffuse + specular;
	color	= vec4(result , 1.0f);
}
```

最终的效果如下：
![specular map](http://img.blog.csdn.net/20160703234222152)
当移动物体时，只有钢架子部分对镜面光反应比较明显，而木制部分则仍然很暗淡。

# 最后的说明

本节通过使用材质属性，模拟不同物体对光有不同反映；使用diffuse map和specular map，模拟物体的不同部分对光有不同反映；同时为光源的三个光成分指定了不同的强度，避免了三个光成分和物体叠加计算后，光照太亮的不真实效果。下一节将会介绍不同的光源，以及使用多个光源的方法。
