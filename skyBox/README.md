写在前面
       之前学习了[2D纹理映射](http://blog.csdn.net/wangdingqiaoit/article/details/51457675)，实际上还有其他类型的纹理有待我们进一步学习，本节将要学习的立方体纹理(cubemaps)，是一种将多个纹理图片复合到一个立方体表面的技术。在游戏中应用得较多的天空包围盒可以使用cubemap实现。本节示例程序均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/skyBox)。

> 本节内容整理自:
1.[Tutorial 25:SkyBox](http://ogldev.atspace.co.uk/www/tutorial25/tutorial25.html)
2.[www.learnopengl.com Cubemaps](http://learnopengl.com/#!Advanced-OpenGL/Cubemaps)

# 创建Cubemap
cubemap是使用6张2D纹理绑定到GL_TEXTURE_CUBE_MAP目标而创建的纹理。GL_TEXTURE_CUBE_MAP包含6个面，分别是:

|绑定目标|纹理方向|
|----------------|-------------|
|GL_TEXTURE_CUBE_MAP_POSITIVE_X |右边|
|GL_TEXTURE_CUBE_MAP_NEGATIVE_X|左边|
|GL_TEXTURE_CUBE_MAP_POSITIVE_Y|顶部|
|GL_TEXTURE_CUBE_MAP_NEGATIVE_Y|底部|
|GL_TEXTURE_CUBE_MAP_POSITIVE_Z |背面|
|GL_TEXTURE_CUBE_MAP_NEGATIVE_Z|前面|


如下图所示，形成一个立方体纹理(来自[Cubemaps]
(https://scalibq.wordpress.com/2013/06/23/cubemaps/))：

![这里写图片描述](http://img.blog.csdn.net/20160911195104146)


**需要注意的是**，OpenGL中相机默认朝向-z方向，因此GL_TEXTURE_CUBE_MAP_NEGATIVE_Z表示前面，而GL_TEXTURE_CUBE_MAP_POSITIVE_Z表示背面。在构建cubemaps，一般利用枚举常量递增的特性，一次绑定到上述6个目标。例如在OpenGL中枚举常量定义为：

```cpp
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
```
可以看到上述6个枚举常量一次递增，我们可以使用循环来创建这个立方体纹理，将这个函数封装到[texture.h](https://github.com/wangdingqiao/noteForOpenGL/blob/master/skyBox/skyBox/texture.h)中如下：

```cpp
   /*
	* 加载一个cubeMap
	*/
static GLuint loadCubeMapTexture(std::vector<const char*> picFilePathVec, 
GLint internalFormat = GL_RGB,
 GLenum picFormat = GL_RGB,
GLenum picDataType = GL_UNSIGNED_BYTE, 
int loadChannels = SOIL_LOAD_RGB)
{
	GLuint textId;
	glGenTextures(1, &textId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textId);
	GLubyte *imageData = NULL;
	int picWidth, picHeight;
for (std::vector<const char*>::size_type  i =0; i < picFilePathVec.size(); ++i)
	{
		int channels = 0;
		imageData = SOIL_load_image(picFilePathVec[i], &picWidth, 
			&picHeight, &channels, loadChannels);
		if (imageData == NULL)
		{
			std::cerr << "Error::loadCubeMapTexture could not load texture file:"
				<< picFilePathVec[i] << std::endl;
			return 0;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, internalFormat, picWidth, picHeight, 0, picFormat, picDataType, imageData);
		SOIL_free_image_data(imageData);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		return textId;
}
```
关于代码中GL_TEXTURE_WRAP_R参数，稍后会做解释。

实际使用时加载6个2D纹理图片，如下所示：
```cpp
	faces.push_back("sky_rt.jpg");
	faces.push_back("sky_lf.jpg");
	faces.push_back("sky_up.jpg");
	faces.push_back("sky_dn.jpg");
	faces.push_back("sky_bk.jpg");
	faces.push_back("sky_ft.jpg");
	GLuint skyBoxTextId = TextureHelper::loadCubeMapTexture(faces);
```
需要注意加载图片的顺序 我们使用GL_TEXTURE_CUBE_MAP_POSITIVE_X + i的方式来一次创建了6个2D纹理，加载图片时的顺序以需要对应枚举变量定义的顺序。

# 使用cubemaps
cubemaps创建了一个立方体纹理，那么如何对纹理进行采样呢？
与2D纹理使用的纹理坐标(s,t)不同，我们这里需要使用三维纹理坐标(s,t,r)，如下图所示(来自[www.learnopengl.com Cubemaps](http://learnopengl.com/#!Advanced-OpenGL/Cubemaps))：

![三维纹理坐标](http://img.blog.csdn.net/20160911201229880)

图中橙色的方向向量，当立方体中心处于原点时，即代表的是立方体表面顶点的位置，这个向量即是三维纹理坐标。利用(s,t,r)决定纹理采样时，首先根据(s,t,r)中模最大的分量决定在哪个面采样，然后使用剩下的2个坐标在对应的面上做2D纹理采样。例如根据(s,t,r)中模最大的为s分量，并且符号为正，则决定选取+x面作为采样的2D纹理，然后使用(t,r)坐标在+x面上做2D纹理采样。关于这个计算过程的解释可以参考[cubemaps](https://scalibq.wordpress.com/2013/06/23/cubemaps/)。

在[2D纹理映射](http://blog.csdn.net/wangdingqiaoit/article/details/51457675)一节我们提到WRAP参数会决定，当纹理坐标超出[0,1]范围时的纹理采样方式。上述代码中，我们使用：

```cpp
glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
```
其中参数GL_CLAMP_TO_EDGE主要用于指定，当(s,t,r)坐标没有落在哪个面，而是落在两个面之间时的纹理采样，使用GL_CLAMP_TO_EDGE参数表明，当在两个面之间采样时使用边缘的纹理值。

# 创建天空包围盒

上面介绍了创建和使用cubemap的方法，实际游戏应用得较多的就是利用cubemap实现天空包围盒。**天空包围盒的主要实现思路**是：
在场景中绘制一个cubemap纹理采样的立方体，将这个立方体总是置于场景中最外围，让游戏玩家感觉到好像场景非常大，触不可及像天空一样，即是玩家靠近一些，天空依然还是离得很远的感觉。
例如下图，我们绘制了一个包围盒：
![包围盒](http://img.blog.csdn.net/20160911203136942)

绘制包围盒，是将1x1x1的立方体作为包围盒，将上面建立的cubemap映射到这个包围盒上。这个立方体的中心处于原点，因此立方体上的顶点位置，就当做前面讲的用于纹理采样的向量。
在顶点着色器中实现为：

```cpp
#version 330 core

layout(location = 0) in vec3 position;


uniform mat4 projection;
uniform mat4 view;
out vec3 TextCoord;
void main()
{
	gl_Position = projection * view * vec4(position, 1.0); 
	TextCoord = position;  // 当立方体中央处于原点时 立方体上位置即等价于向量
}
```
在片元着色器中只需要采样纹理即可:

```cpp
   #version 330 core

in vec3 TextCoord;
uniform samplerCube  skybox;  // 从sampler2D改为samplerCube
out vec4 color;

void main()
{
	color = texture(skybox, TextCoord);
}
```
要将包围盒置于场景中，最外层，基本的方式是，暂时关闭深度缓存写入，首先绘制包围盒，这样包围盒总是处于场景中最外围。同时实现的时候**需要注意是如何保持玩家移动时，包围盒看起来很远很大的感觉，有两种实现方式。**

**第一种方式**，去掉视变换中移动的部分(translate部分)，但保留旋转等其他成分，这样当你在场景内移动，转动相机时，包围盒仍然在以正常角度显示，只是包围盒没有因为玩家的前进后退而发生移动，这样看起来就比较正常。这种方式实现为：
```cpp
   // 先绘制skyBox
glDepthMask(GL_FALSE); // 禁止写入深度缓冲区
skyBoxShader.use();
glm::mat4 projection = glm::perspective(camera.mouse_zoom,
	(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f); // 投影矩阵
glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix())); // 视变换矩阵 移除translate部分
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "view"),1, GL_FALSE, glm::value_ptr(view));
```

**第二种方式**，是每次将包围盒的中心设定在玩家的位置，同时以一定比例缩放包围盒，这样达到的效果基本相同，但是缺点是如果缩放比例不当的话，场景中物体移动时可能超出包围盒，而引起视觉Bug。这种方式实现为：

```cpp
skyBoxShader.use();

glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "projection"),1, GL_FALSE, glm::value_ptr(projection));	

glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "view"),1, GL_FALSE, glm::value_ptr(view));

model = glm::translate(glm::mat4(), camera.position);

model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "model"),1, GL_FALSE, glm::value_ptr(model));
```

# 天空包围盒的改进
上面在绘制天空包围盒时，我们首先关闭深度缓存写入，绘制包围盒，让它处于场景最外围，这样做当然能正常工作，缺点是如果场景中物体需要显示在包围盒前面，最终包围盒的某些部分会被遮挡住，按上述绘制方式我们还是绘制了这部分内容，导致了不必要的着色器调用，这是一种性能上的损失。

一种改进的策略是首先绘制场景中物体，然后根据利用包围盒的深度值和当前深度值进行比较，如果通过深度测试就绘制包围盒。我们知道默认情况下，清除深度缓存时使用的值为1.0表示深度最大，因此我们也想用1.0来表示包围盒的深度值，这样它就始终处于场景中最外围，当进行深度测试时，我们改变默认的测试函数，从GL_LESS变为GL_LEQUAL，如下：

```cpp
   glDepthFunc(GL_LEQUAL); // 深度测试条件 小于等于
```

那么如何让包围盒的深度值总是1.0呢？ 我们知道，在顶点着色器中，gl_Position表示的是当前顶点的裁剪坐标系坐标(对应的z分量为$Z_{clip}$)，而一个顶点最终的深度值是通过透视除法得到NDC坐标(对应的z分量为$Z_{ndc}$)，以及最后的视口变换后得到窗口坐标的$Z_{win}$值决定的。关于这个深度值的计算，如果感觉陌生，可以回过头去查看[深度测试一节](http://blog.csdn.net/wangdingqiaoit/article/details/52206602)。这里使用的技巧是，手动将gl_Position的z值设定为w，即在顶点主色器中输出：

```cpp
   void main()
{
	vec4 pos = projection * view * model * vec4(position, 1.0); 
	gl_Position = pos.xyww;  // 此处让z=w 则对应的深度值变为depth = w / w = 1.0
	TextCoord = position;  // 当立方体中央处于原点时 立方体上位置即等价于向量
}
```
这样通过OpenGL默认执行的透视除法和视口变换后，得到的深度值就是$Z_{win}=1.0$，达到我们的目的。这种方式首先绘制场景中物体，最后渲染包围盒。需要注意的是，绘制包围盒时将深度测试函数变为:

```cpp
   glDepthFunc(GL_LEQUAL);
```
绘制完毕后，又恢复默认的GL_LESS。使用不同的包围盒素材，我们得到另一个包围盒效果如下图所示：

![包围盒2](http://img.blog.csdn.net/20160911220204857)


# 最后的说明
在实现包围盒时，需要通过移除translate部分(上文中第一种方式)或者将包围盒设为观察者原点,并且放大包围盒的方式(上文中第二种方式)来使包围盒看起来很远很大。如果设置不当得到的错误效果可能如下：
![错误效果](http://img.blog.csdn.net/20160911220618953)

在实现包围盒时，注意调整合适的投影变换参数，这里我们设置的参数为:
```cpp
   glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f); // 投影矩阵
```
如果投影参数设置不当，得到错误的效果可能如下：
![投影参数不当](http://img.blog.csdn.net/20160911220406593)

要想获得更多的包围盒，可以访问[在线资源](http://www.custommapmakers.org/skyboxes.php)。

另外Cubemap还可以用来实现environment mapping等技术，下一节将会学习这个主题。

# 参考资料
1.https://scalibq.wordpress.com/2013/06/23/cubemaps/
2.https://www.opengl.org/wiki/Skybox
3.http://learnopengl.com/#!Advanced-OpenGL/Cubemaps
4.http://www.c-jump.com/bcc/common/Talk3/OpenGL/Wk13_skybox/Wk13_skybox.html