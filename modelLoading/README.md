写在前面
[上一节Obj模型加载](http://blog.csdn.net/wangdingqiaoit/article/details/51879737)作为模型加载的基础篇，介绍了模型中Mesh的概念，以及从Obj模型文件转换为OpenGL数据格式的方法。上一节绘制的结果还是很简陋，本节将给出上节承诺的酷炫的3D模型的加载方法:)。本节示例程序均可以在[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/modelLoading)。

> 本节内容参考自
   1.[www.learnopengl.com Model loading](http://www.learnopengl.com/#!Model-Loading/Model)
   2.[Loading models using the Open Asset Import Library ](http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html)

# 下载和安装AssImp

[AssImp](http://assimp.sourceforge.net/lib_html/index.html)是一个模型加载库，它将不同格式的模型数据转换为统一的抽象的数据类型，因而支持较多的模型文件格式。下载和编译这个库的过程，你可以参考[官方文档](http://www.assimp.org/lib_html/install.html)。这里也为Windows用户提供了编译好的lib和dll文件，可以从[我的github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/libraries)。

# AssImp的数据结构
AssImp的主要的类如下图所示：

![类图](http://img.blog.csdn.net/20160724153549638)

这里对上述图作简要说明：

- aiScene  作为AssImp加载模型的根数据结构，保存了从模型加载的顶点位置、法向量、纹理、光照等数据，例如它通过数组mMeshes保存Mesh数据，通过mMaterials保存材质数据。总之aiScene保存了加载的模型数据，其余类通过索引aiScene中存储的对象来获取对应的数据。

- aiNode 模型通过层次结构存储，根节点mRootNode保存在aiScene中，根节点下面有0至多个子节点，每个节点通过aiNode类表达，aiNode中包含一个或者多个Mesh，注意这里的Mesh是对aiScene中Mesh数据的一个索引。

- aiMesh是上一节中所讲的Mesh对象，Mesh中包含顶点位置数据、法向量、纹理数据，每个Mesh可以包含一个或者多个Face。

- aiFace是一个面，一般来讲在读取模型时通过后处理选项(post-process flag)将模型转换为三角形网格，那么这里的面主要是三角形面。后处理选项稍后介绍。通过三角形面，我们可以获取渲染模型需要的索引数据。

# OpenGL需要的数据结构
加载模型的任务就是将抽象的模型数据转换为OpenGL可以处理的VBO,EBO,纹理数据。在程序内部我们定义了Mesh，Model结构来作为内部格式。Mesh表达是绘制的最小实体，它包含顶点属性数据、材质数据；Model则是包含1个或者多个Mesh的模型。定义Mesh结构如下：
```C++
// 表示一个顶点属性
struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
};

// 表示一个Texture
struct Texture
{
	GLuint id;
	aiTextureType type;
	std::string path;
};

// 表示一个用于渲染的最小实体
class Mesh
{
public:
	void draw(const Shader& shader) const;// 绘制Mesh
	Mesh():VAOId(0), VBOId(0), EBOId(0){}
	Mesh(const std::vector<Vertex>& vertData, 
		const std::vector<Texture> & textures,
		const std::vector<GLuint>& indices); // 构造一个Mesh
	void final() const; // 释放VBO等空间
private:
	std::vector<Vertex> vertData;  // 顶点属性数据
	std::vector<GLuint> indices;	// 索引数据	
	std::vector<Texture> textures;  // 纹理数据
	GLuint VAOId, VBOId, EBOId;
	void setupMesh();  // 建立VAO,VBO等缓冲区
};
```
为了简化程序，这里我们只处理了材质中的纹理数据。Model则是一个包含多个Mesh的类，定义如下：

```C++
/*
* 代表一个模型 模型可以包含一个或多个Mesh
*/
class Model
{
public:
	void draw(const Shader& shader) const
	{
		for (mesh in meshes)
		{
			mesh->draw(shader);
		}
	}
	bool loadModel(const std::string& filePath);
	~Model()
	{
		for (mesh in meshes)
		{
			mesh->final();
		}
	}
private:
	bool processNode(const aiNode* node, const aiScene* sceneObjPtr); 
	bool processMesh(const aiMesh* meshPtr, const aiScene* sceneObjPtr, Mesh& meshObj);
	bool processMaterial(const aiMaterial* matPtr, 
		const aiScene* sceneObjPtr, Material& material);
private:
	std::vector<Mesh> meshes; // 保存Mesh
	std::string modelFileDir; // 保存模型文件的文件夹路径
	typedef std::map<std::string, Texture> LoadedTextMapType; // key = texture file path
	LoadedTextMapType loadedTextureMap; // 保存已经加载的纹理
};
```
需要注意的是模型文件所在文件路径我们通过modelFileDir保存起来，因为模型中纹理数据可能使用相对路径来表示纹理，通过modelFileDir加上这个相对路径才能找到纹理图片的正确路径。


# AssImp加载模型
加载模型时首先创建 Assimp::Importer的示例，然后通过它的l Assimp::Importer::ReadFile()方法加载模型，如下所示：
```C++
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Assimp::Importer importer;
const aiScene* sceneObjPtr = importer.ReadFile(filePath, 
			aiProcess_Triangulate | aiProcess_FlipUVs);
if (!sceneObjPtr
	|| sceneObjPtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE
	|| !sceneObjPtr->mRootNode)
{
	std::cerr << "Error:Model::loadModel, description: " 
		<< importer.GetErrorString() << std::endl;
	return false;
}
```
ReadFile函数中第二个参数就是后处理选项，它是一个枚举类型aiPostProcessSteps，可以使用位或操作包含多个选项，例如选项aiProcess_MakeLeftHanded表示将默认的右手系坐标数据转换为左手系坐标数据，aiProcess_Triangulate选项将索引数据多余3个的多边形划分为多个三角形，方便我们使用三角形进行绘制。完整的后处理选项列表，可以参考[官方文档](http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410)。

通过上面的加载我们获取到了模型的根结构数据aiScene，**接下来的工作就是:从aiScene获取OpenGL所需要的VBO,EBO,纹理数据。**如果对此完全没有概念，建议你回到[上一节Obj模型加载](http://blog.csdn.net/wangdingqiaoit/article/details/51879737)了解下这种转换的概念。

AssImp中数据通过aiNode组织父子结点，包含了层次信息，我们可以忽略这些信息，直接读取所有我们需要的VBO,EBO，纹理数据，但是这种父子结构信息在后面制作骨骼动画时会再次用到，因此这里还是按照层次的方式来解析aiScene数据。

所谓结点就是包含一个多个Mesh的部位，例如一个人物角色，可能包含头部，颈部，手臂，胸部等多个结点，每个结点也可以包含更多的细化结点。解析aiScene这种父子结点的层次数据，直观的方法就是使用递归，递归就是一个函数直接调用自己，一层一层调用下去，当遇到一个合适条件时终止调用，函数一层层返回。如果你想了解更多地了解递归，可以参考[我关于递归方法的博客](http://blog.csdn.net/wangdingqiaoit/article/details/41627581)。从aiScene解析模型数据获取OpenGL所需数据的框架大概是这样的:

```C++
bool loadModel(const std::string& filePath)
{
	// 加载模型 得到aiScene
const aiScene* sceneObjPtr = importer.ReadFile(filePath, 
	aiProcess_Triangulate | aiProcess_FlipUVs);
	// 递归处理结点
	return this->processNode(sceneObjPtr->mRootNode, sceneObjPtr);
}

bool processNode(const aiNode* node, const aiScene* sceneObjPtr)
{
	
	for (size_t i = 0; i < node->mNumMeshes; ++i) // 先处理自身结点
	{
		// 注意node中的mesh是对sceneObject中mesh的索引
		const aiMesh* meshPtr = sceneObjPtr->mMeshes[node->mMeshes[i]]; 
		this->processMesh(meshPtr, sceneObjPtr, meshObj); // 处理Mesh
	}
	
	for (size_t i = 0; i < node->mNumChildren; ++i) // 再处理孩子结点
	{
		this->processNode(node->mChildren[i], sceneObjPtr);
	}
	return true;
}
bool processMesh(const aiMesh* meshPtr, const aiScene* sceneObjPtr, Mesh& meshObj)
{
	// 从Mesh得到顶点数据、法向量、纹理数据
	for (size_t i = 0; i < meshPtr->mNumVertices; ++i){...}
	// 获取索引数据
	for (size_t i = 0; i < meshPtr->mNumFaces; ++i){...}
	 // 获取纹理数据
	if (meshPtr->mMaterialIndex >= 0)
	{
		this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_DIFFUSE, diffuseTexture);
		this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_SPECULAR, specularTexture);
	}
	return true;
}
```

上面的框架给出了从aiScene获取数据，建立内部格式Model和Mesh的思路，具体实现细节可以[参考程序源码](https://github.com/wangdingqiao/noteForOpenGL/tree/master/modelLoading/AssImplLoad)。

# 加载纳米战斗服模型
到这里，我们可以来欣赏酷炫的模型了，首先加载一个从[learnopengl](http://www.learnopengl.com/#!Model-Loading/Model)获取的纳米战斗服模型nanosuit，效果如下图所示：
![没有光照效果](http://img.blog.csdn.net/20160724163945134)

这里没有使用光照，下面是实现了一个点光源的效果：

![有光照效果](http://img.blog.csdn.net/20160726153834123)
可以从机器人胸部的高光部分看到，实现光照时的区别。

# 加载海岛模型

下图是加载了海岛模型SmallTropicalIsland的效果：
![海岛模型](http://img.blog.csdn.net/20160724164316905)


# 加载城市建筑模型

下图是加载了城市模型TheCity的效果：
![城市模型](http://img.blog.csdn.net/20160724164416234)
使用了一个手电筒聚光灯模型的城市建筑模型，效果如下(你可以作为练习，这个项目源码未提供)：
![城市模型-聚光灯](http://img.blog.csdn.net/20160724165404247)

上面加载的模型都可以通过WASD和配合鼠标查看，在海岛和城市建筑模型中通过第一人称相机来漫游场景，感觉还是很不错的。

# 加载模型需要注意的地方
1.加载模型后，需要适当设置模型变换矩阵，否则模型显示在奇怪的位置。这个模型变换矩阵，目前还没找到合适的方法从模型数据中获取。
2.下载的模型，有些路径是不正确的，本文统一采用绝对路径方式。路径不正确或者文件缺失时的错误提示，例如：
![纹理加载错误](http://img.blog.csdn.net/20160724170539171)
加载错误的原因一部分要归咎于原作者，使用了诸如users,C:\等本地盘符，如果你从网上下载3D模型后，请记住检查文件路径。
3.部分纹理图片的格式，模型的格式目前并未处理，不支持加载。

# 还需要改进的地方
上面加载的模型，已经让人很兴奋了，但是还不够真实，高效。在实验过程中，思考还需要通过以下方面进行改进：

1.我们这里的材质只处理了纹理部分，实际上模型中如果没有通过纹理定义材质，还需要获取ambient等颜色表示的材质。而且纹理可能不止一个，本文目前只处理了一个纹理(主要原因是下载的素材里面没有找不到更多的纹理坐标)。可以通过定义下面的材质结构体，并处理这个材质数据来丰富场景：
```C++
   // 表示材质属性
struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
	std::vector<Texture> textures;
};
```

2.模型中要通过光源和相机加以改善。目前在模型中通过以下方式：
```C++
if (sceneObjPtr->HasLights() 
&& !this->processLightSource(sceneObjPtr))
{
std::cerr 
<< "Error:Model::loadModel, process lights failed."
 << std::endl;
	return false;
}
```
获取光源数据时，大量从网络上下载的模型中并没有找到光源数据，比较可惜。

3.实际模型的材质中包含了map_Bump数据，但目前还未学习处理方法。
4.目前通过Model加载模型时耗时非常多，效率不高，需要进一步提高模型加载和渲染的速度。

