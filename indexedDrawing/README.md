写在前面
           [上一节](http://blog.csdn.net/wangdingqiaoit/article/details/51318793)熟悉了OpenGL绘图的中基本的VAO和VBO，以及着色器，并绘制了一个三角形。本节在上一节基础上，介绍利用EBO的索引绘图。

通过本节可以了解到

- 索引绘图使用方法
- 索引绘图应用场合

## 使用EBO开始索引绘图
EBO即Element Buffer Objects，用来存储绘制物体的索引。所谓索引就是对顶点属性数组中元素的一个位置标记。使用索引绘图时，先通过顶点属性数组指定数据，然后利用指向这些数据的索引来指示OpenGL完成绘图。使用索引绘图的原理如下图所示(来自[VBO Indexing](http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/)):
![这里写图片描述](http://img.blog.csdn.net/20160505163820379)

在左边图中，没有使用索引绘制矩形时，要重复指定重叠的顶点数据，V1和V2都重复了，使用了6个顶点属性数据；右边的图中，使用索引绘制时，只需要指定顶点在属性数组中的索引即可，使用0,1,2,3代表V0,V1,V2,V3顶点，绘制矩形一共指定了6个索引，使用4个顶点属性数据。

上面的例子可能看不出有大区别，但是当要绘制的物体包含多个重叠的顶点时，如果每个顶点属性包括了位置、颜色、纹理坐标、法向量等属性，那么将会造成很大的额外空间开销，影响GPU性能。同时，如果需要修改一个顶点处的数据，那么重复的顶点数据都要被改过来，这个工作也很无趣。因此，使用索引绘制能够节省存储空间，而且能灵活应对顶点属性的改变。
不使用索引也可以绘制矩形，我们使用6个顶点，数据如下:
```C++
GLfloat vertices[] = {
		// 第一个三角形
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
		// 第二个三角形
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	};
```
然后在绘制时更新顶点数目即可:
```C++
	glDrawArrays(GL_TRIANGLES, 0, 6);
```
下面介绍使用索引的绘制方法。我们需要在VBO中存储顶点属性数据，这在上一节已经介绍了。另一方面，就是使用EBO存储索引数据。我们重新指定顶点数据和索引数据如下：
```C++
   // 指定顶点属性数据 顶点位置 颜色
	GLfloat vertices[] = {
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 0
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 1
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // 2
		0.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f  // 3
	};
	// 索引数据
	GLshort indices[] = {
		0, 1, 2,  // 第一个三角形
		0, 3, 1   // 第二个三角形
	};
```
创建EBO，并将索引数据传送到EBO，如下:
```c++
glGenBuffers(1, &EBOId);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOId);
glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	sizeof(indices), indices, GL_STATIC_DRAW);
```
顶点属性数据部分无需改动，在绘制物体时需要将glDrawArrays调整为[glDrawElements](https://www.opengl.org/sdk/docs/man/html/glDrawElements.xhtml).
> **API** void glDrawElements( 	GLenum mode,
  	GLsizei count,
  	GLenum type,
  	const GLvoid * indices);
  	1.参数mode 同glDrawArrays一样表示绘制的基本图元类型  GL_POINTS, GL_LINE_STRIP等。
  	2.参数count表示使用的EBO中索引元素的个数。
  	3.参数type 表示索引数据的数据类型。必须取 GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, 或者 GL_UNSIGNED_INT 三者之一。
  	4.indices 表示EBO中索引的偏移量。

这里需要基本图元类型为GL_TRIANGLES,使用6个索引，索引类型为GL_UNSIGNED_SHORT, 同时索引数据在EBO中起点处，indices参数为0. 绘制代码为:
```C++
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
```
完整的代码可以[从github下载](https://github.com/wangdingqiao/noteForOpenGL/tree/master/indexedDrawing)。
运行结果如下图所示:
![这里写图片描述](http://img.blog.csdn.net/20160505165842137)

## 索引绘制的应用场合
从上面可以看出索引绘制能够节省存储空间，共享顶点属性数据，但存在的限制时共享的数据的属性时相同的。当我们需要为同一个顶点指定不同的属性，例如颜色和法向量时，索引绘制无法满足需求，这时候需要使用顶点数组为同一个顶点指定不同属性。

## 推荐阅读
1.[Drawing Cube with indices ](http://in2gpu.com/2015/07/09/drawing-cube-with-indices/)
2.[An Introduction to Index Buffer Objects](http://www.learnopengles.com/android-lesson-eight-an-introduction-to-index-buffer-objects-ibos/)

##参考资料
1. [www.learnopengl.com](http://www.learnopengl.com/#!Getting-started/Hello-Triangle)
2. [Indexed Draws](http://www.ogldev.org/www/tutorial10/tutorial10.html) 