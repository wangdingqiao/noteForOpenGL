写在前面
前面介绍了[光照基础内容](http://blog.csdn.net/wangdingqiaoit/article/details/51638260)，以及[材质和lighting maps](http://blog.csdn.net/wangdingqiaoit/article/details/51817860)，和[光源类型](http://blog.csdn.net/wangdingqiaoit/article/details/51867538)，我们对使用光照增强场景真实感有了一定了解。但是到目前为止，我们通过在程序中指定的立方体数据，绘制立方体，看起来还是很乏味。本节开始介绍模型加载，通过加载丰富的模型，能够丰富我们的场景，变得好玩。本节的示例代码均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/modelLoading)。

加载模型可以使用比较好的库，例如[obj模型加载的库](https://github.com/syoyo/tinyobjloader)，[Assimp加载库](http://assimp.sourceforge.net/lib_html/index.html)。本节作为入门篇，我们一开始不使用这些库加载很酷的模型，而是熟悉下模型以及模型加载的概念，然后我们封装一个简单的obj模型加载类，加载一个简单的立方体模型。

**不要太急于看到漂亮的3D模型，下一节我们会使用Assimp库会加载一个酷炫的3d模型，但是本节还是注重多感受下模型加载的基础，否则下一节学习起来会吃力。**


通过本节可以了解到

- Mesh的概念
- Obj模型数据格式
- Obj模型简单的加载类和加载实验


## 模型的表达

在3d图形处理中，一个模型(model)通常由一个或者多个Mesh（网格）组成，一个Mesh是可绘制的独立实体。例如复杂的人物模型，可以分别划分为头部，四肢，服饰，武器等各个部分来建模，这些Mesh组合在一起最终形成人物模型。

Mesh由顶点、边、面Faces组成的，它包含绘制所需的数据，例如顶点位置、纹理坐标、法向量，材质属性等内容，它是OpenGL用来绘制的最小实体。Mesh的概念示意如下图所示(来自:[What is a mesh in OpenGL?](https://www.quora.com/What-is-a-mesh-in-OpenGL))：

![Mesh](http://img.blog.csdn.net/20160711230322418)

Mesh可以包含多个Face，一个Face是Mesh中一个可绘制的基本图元，例如三角形，多边形，点。要想模型更加逼真，一般需要增加更多图元使Mesh更加精细，当然这也会受到硬件处理能力的限制，例如PC游戏的处理能力要强于移动设备。由于多边形都可以划分为三角形，而三角形是图形处理器中都支持的基本图元，因此使用得较多的就是三角形网格来建模。例如下面的图(来自:[What is a mesh in OpenGL?](https://www.quora.com/What-is-a-mesh-in-OpenGL))表达了使用越来越复杂的Mesh建模一只兔子的过程：

![Mesh2](http://img.blog.csdn.net/20160711230453733)

随着增加三角形个数，兔子模型变得越来越真实。

目前模型存储的格式很丰富，比较常用的，例如[Wavefront .obj file](https://en.wikipedia.org/wiki/Wavefront_.obj_file)，[COLLADA](https://en.wikipedia.org/wiki/COLLADA)等，要了解各个格式的特点，可以参考[wiki 3D graphics file formats](https://en.wikipedia.org/wiki/Category:3D_graphics_file_formats)。在众多的格式中以obj格式比较通用，它内部是以文本形式表达的，接下来我们通过熟悉下obj格式，了解模型是如何定义的，以及如何加载到OpenGL中来渲染模型。

# Obj模型数据格式
obj模型内部以文本存储，例如从[Model loading](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/)处获取的一个立方体模型cube.obj的数据如下：
```
# Blender3D v249 OBJ File: untitled.blend
# www.blender3d.org
mtllib cube.mtl
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -1.000000
v 0.999999 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
vt 0.748573 0.750412
vt 0.749279 0.501284
vt 0.999110 0.501077
vt 0.999455 0.750380
vt 0.250471 0.500702
vt 0.249682 0.749677
vt 0.001085 0.750380
vt 0.001517 0.499994
vt 0.499422 0.500239
vt 0.500149 0.750166
vt 0.748355 0.998230
vt 0.500193 0.998728
vt 0.498993 0.250415
vt 0.748953 0.250920
vn 0.000000 0.000000 -1.000000
vn -1.000000 -0.000000 -0.000000
vn -0.000000 -0.000000 1.000000
vn -0.000001 0.000000 1.000000
vn 1.000000 -0.000000 0.000000
vn 1.000000 0.000000 0.000001
vn 0.000000 1.000000 -0.000000
vn -0.000000 -1.000000 0.000000
usemtl Material_ray.png
s off
f 5/1/1 1/2/1 4/3/1
f 5/1/1 4/3/1 8/4/1
f 3/5/2 7/6/2 8/7/2
f 3/5/2 8/7/2 4/8/2
f 2/9/3 6/10/3 3/5/3
f 6/10/4 7/6/4 3/5/4
f 1/2/5 5/1/5 2/9/5
f 5/1/6 6/10/6 2/9/6
f 5/1/7 8/11/7 6/10/7
f 8/11/7 7/12/7 6/10/7
f 1/2/8 2/9/8 3/13/8
f 1/2/8 3/13/8 4/14/8

```
对这个文本格式做一个简要说明：

- 以#开始的行为注释行
- usemtl和mtllib表示的纹理相关数据，解析纹理数据稍微繁琐，本节我们只是为了说明加载模型的原理，不做讨论。
- o 引入一个新的object
- v 表示顶点位置
- vt 表示顶点纹理坐标
- vn 表示顶点法向量
- f  表示一个面，面使用1/2/8这样格式，表示顶点位置/纹理坐标/法向量的索引，这里索引的是前面用v,vt,vn定义的数据 注意这里Obj的索引是从1开始的，而不是0

模型一般通过3d建模软件，例如[Blender](http://www.blender.org/), [3DS Max](http://www.autodesk.nl/products/3ds-max/overview) 或者 [Maya](http://www.autodesk.com/products/autodesk-maya/overview)等工具建模，导出时的数据格式变化较大，**我们导入模型到OpenGL的任务就是**：将一种模型数据文件表示的模型，转换为OpenGL可以利用的数据。例如上面的Obj文件中，我们需要解析顶点位置，纹理坐标等数据，构成OpenGL可以渲染的Mesh对象。 

# 从Obj到OpenGL可以理解的Mesh

上面说明了Obj的数据格式，那么在OpenGL中我们怎么表达Mesh呢？首先定义顶点属性数据如下所示：
```C++
 // 表示一个顶点属性
struct Vertex
{
	glm::vec3 position;  // 顶点位置
	glm::vec2 texCoords; // 纹理坐标
	glm::vec3 normal;  // 法向量
};
```
Mesh中包含顶点属性，纹理对象等信息，本节我们定义Mesh数据结构如下所示：
```C++
  
// 表示一个OpenGL渲染的最小实体
class Mesh
{
public:
	void draw(Shader& shader) // 绘制Mesh
	Mesh(const std::vector<Vertex>& vertData, 
		GLint textureId) // 构造一个Mesh
private:
	std::vector<Vertex> vertData;// 顶点数据
	GLuint VAOId, VBOId; // 缓存对象
	GLint textureId; // 纹理对象id
	void setupMesh();  // 建立VAO,VBO等缓冲区
};
```

载入obj模型的过程，就是读取obj文件，并转换为上面Mesh对象的过程。这个过程的思路大致是这样的，读取文件的每一行，根据行首部的指示，确定数据类型，然后加载到mesh的vertData里面去，这个框架是这样：
```C++
std::ifstream file(objFilePath);
while (getline(file, line))
{
	if (line.substr(0, 2) == "vt") // 顶点纹理坐标数据
	{
		// 解析顶点纹理数据
	}
	else if (line.substr(0, 2) == "vn") // 顶点法向量数据
	{
		// 解析法向量数据
	}
	else if (line.substr(0, 1) == "v") // 顶点位置数据
	{
		// 解析顶点位置数据
	}
	else if (line.substr(0, 1) == "f") // 面数据
	{
		// 解析面数据
	}
	else if (line[0] == '#') // 注释忽略
	{ }
	else  
	{
		// 其余内容 暂时不处理
	}
}
```
上面提供了一个读取obj文件格式的框架，例如解析纹理数据如下：
```C++
if (line.substr(0, 2) == "vt") // 顶点纹理坐标数据
{
	std::istringstream s(line.substr(2));
	glm::vec2 v;
	s >> v.x; 
	s >> v.y;
	v.y = -v.y;  // 注意这里加载的dds纹理 要对y进行反转
	temp_textCoords.push_back(v);
}
```
其余的也类似处理。读取到数据后，在Mesh对象里面需要向前面绘制物体时一样建立缓冲数据，如下：
```C++
void setupMesh()  // 建立VAO,VBO等缓冲区
{
	glGenVertexArrays(1, &this->VAOId);
	glGenBuffers(1, &this->VBOId);

	glBindVertexArray(this->VAOId);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)* this->vertData.size(),
		&this->vertData[0], GL_STATIC_DRAW);
	// 顶点位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点纹理坐标
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	// 顶点法向量属性
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (GLvoid*)(5 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
```
建立缓冲区的同时，本节我们使用的立方体模型cube.dds纹理如下图所示：
![dds](http://img.blog.csdn.net/20160711234637189)

这与以前使用的png纹理不一样，这里我用C++重新改编了[Model loading](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/)处的加载dds纹理的函数，加载纹理不是本节的重点，具体可以查看[github代码](https://github.com/wangdingqiao/noteForOpenGL/blob/master/modelLoading/simpleObjLoader/texture.h)。加载纹理后，可以渲染这个obj表达的立方体模型，整个过程如下：

```C++
//Section1 从obj文件加载数据
std::vector<Vertex> vertData;
ObjLoader::loadFromFile("cube.obj", vertData)

// Section2 准备纹理
GLint textureId = TextureHelper::loadDDS("cube.dds");

// Section3 建立Mesh对象
Mesh mesh(vertData, textureId);

// Section4 准备着色器程序
Shader shader("cube.vertex", "cube.frag");

// 在游戏主循环中渲染立方体
```
这里我们可以看到，与以往在程序中通过数值指定立方体模型相比，我们的代码更简洁，后面介绍使用[Assimp加载库](http://assimp.sourceforge.net/lib_html/index.html)后，可以加载更多丰富的模型，当然要比这个立方体好看。但是本节还是看一下最终立方体的效果吧，如下：
![obj加载](http://img.blog.csdn.net/20160711235223593)

# 最后的说明

在使用dds纹理的时候，要注意纹理的y轴相对于OpenGL是进行反转的，因此需要使用( coord.u, 1.0-coord.v) 来访问，这可以在加载obj时做，也可以在着色器里面做。没有使用反转的v坐标将导致，无法正常渲染，这也是困住我的一个地方。后来使用数据比对格式发现了这个错误，如下图，左边是反转了的数据，右边是未反转的数据：

![v导致的错误](http://img.blog.csdn.net/20160711235814428)

在使用[blender](https://www.blender.org/)软件导出模型时，即使勾选了includ UVs，输出时仍然没有纹理坐标，这是因为除了勾选这些选项外，还需要一个uv map操作，关于这一点也是容易产生错误的，详细可以参考[Add UV Mapped texture coordinates to OBJ file?](http://blender.stackexchange.com/questions/18422/add-uv-mapped-texture-coordinates-to-obj-file)。uv mappring这个操作的过程比较繁琐，就不再这里介绍了，感兴趣地可以参考[UV Mapping a Mesh](https://www.blender.org/manual/editors/uv_image/uv_editing/unwrapping.html)

最后本节的加载obj程序只是一个示例，并没有解析纹理mtl部分，不使用纹理数据绘制经典的Suzanne 模型如下图所示：
![Suzanne](http://img.blog.csdn.net/20160712000608025)

这里缺少了纹理和光照，所以模型看起来不真实，下一节介绍使用[Assimp加载库](http://assimp.sourceforge.net/lib_html/index.html)时将会改善这一点。

# 参考资料
1. https://www.quora.com/What-is-a-mesh-in-OpenGL
2. http://gamedev.stackexchange.com/questions/38412/whats-the-difference-between-mesh-and-a-model
3. http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
4. https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Load_OBJ
5. http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/