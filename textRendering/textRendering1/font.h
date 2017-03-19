#ifndef _OPENGL_FONT_H_
#define _OPENGL_FONT_H_

#include <GLEW/glew.h>
#include <ft2build.h>
#include <GLM/glm.hpp>
#include <map>
#include <string>
#include <iostream>

#include FT_FREETYPE_H

struct FontCharacter
{
	GLuint textureId; // 存储字形纹理
	glm::ivec2 size;  // 字形大小
	glm::ivec2 bearing; // bearingx bearingy
	GLuint advance;  // 距离下个字形的水平距离
};

class FontResourceManager
{
private:
	typedef std::map<std::string, FT_Face> FontFaceMapType;
public:
	static FontResourceManager& getInstance()
	{
		static FontResourceManager instance;
		return instance;
	}
	~FontResourceManager()
	{
		for (FontFaceMapType::const_iterator it = _faceMap.begin(); it != _faceMap.end(); ++it)
		{
			if (it->second)
			{
				FT_Done_Face(it->second);
			}
		}
		FT_Done_FreeType(_ft);
	}
private:
	FontResourceManager()
	{
		if (FT_Init_FreeType(&_ft))
		{
			std::cerr << "ERROR::FontResourceManager: Could not init FreeType Library" << std::endl;
		}	
	}
	FontResourceManager(FontResourceManager const&){} // 单例模式不实现
	void operator=(FontResourceManager const&){} // 单例模式不实现
public:
	void loadFont(const std::string& fontName, const std::string& fontPath)
	{
		if (NULL != getFontFace(fontName))
		{
			return;
		}
		FT_Face face;
		if (FT_New_Face(_ft, fontPath.c_str(), 0, &face))
		{
			std::cerr << "ERROR::FontResourceManager: Failed to load font with name=" 
				<< fontPath << std::endl;
			return;
		}
		_faceMap[fontName] = face;
		FT_Select_Charmap(face, FT_ENCODING_UNICODE); // unicode编码
	}
	bool loadASCIIChar(const std::string& fontName, const int fontSize,
		std::map<wchar_t, FontCharacter>& characters)
	{
		// 因为我们使用1个字节的颜色分量来存储纹理 所以解除默认的4字节对齐限制
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FT_Face face = getFontFace(fontName);
		if (NULL == face)
		{
			std::cerr << "ERROR::FontResourceManager: Failed to get font with name="
				<< fontName << std::endl;
			return false;
		}
		FT_Set_Pixel_Sizes(face, 0, fontSize);
		for (GLubyte c = 0; c < 255; c++)
		{
			if (!isprint(c))
			{
				continue;
			}
			// 加载字符的字形 
			// FT_LOAD_RENDER  选项告知freeType创建一个 8-bit grayscale bitmap image
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cerr << "ERROR::FontResourceManager: Failed to load Glyph with char=" << c << std::endl;
				continue;
			}
			// 为字体对应的字形创建一个纹理 保存以备后续处理
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
				);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			FontCharacter character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			characters.insert(std::pair<GLchar, FontCharacter>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}
	bool loadUnicodeText(const std::string& fontName, const int fontSize,
		const std::wstring& text,
		std::map<wchar_t, FontCharacter>& characters)
	{
		// 因为我们使用1个字节的颜色分量来存储纹理 所以解除默认的4字节对齐限制
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FT_Face face = getFontFace(fontName);
		if (NULL == face)
		{
			std::cerr << "ERROR::FontResourceManager: Failed to get font with name="
				<< fontName << std::endl;
			return false;
		}
		FT_Set_Pixel_Sizes(face, 0, fontSize);
		for (std::wstring::const_iterator it = text.begin(); it != text.end(); ++it)
		{
			wchar_t wchar = *it;
			// 加载字符的字形 
			// FT_LOAD_RENDER  选项告知freeType创建一个 8-bit grayscale bitmap image
			if (FT_Load_Char(face, wchar, FT_LOAD_RENDER))
			{
				std::cerr << "ERROR::FontResourceManager: Failed to load Glyph with char=" << wchar << std::endl;
				continue;
			}
			// 为字体对应的字形创建一个纹理 保存以备后续处理
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
				);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			FontCharacter character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			characters.insert(std::pair<wchar_t, FontCharacter>(wchar, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}
private:
	FT_Face getFontFace(const std::string& fontName)
	{
		if (_faceMap.count(fontName) > 0)
		{
			return _faceMap[fontName];
		}
		return NULL;
	}
private:
	FT_Library _ft;
	FontFaceMapType _faceMap;
};


#endif