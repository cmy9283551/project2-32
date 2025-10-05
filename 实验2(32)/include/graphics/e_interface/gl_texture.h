#pragma once

#include <string>
#include <vector>

#include "graphic_tool.h"

class Texture2DData {
public:
	Texture2DData(const std::string& file_path);
	~Texture2DData();
	int width()const;
	int height()const;
	int channels()const;
	const std::vector<unsigned char>& data()const;
private:
	std::vector<unsigned char> data_;
	int width_, height_, channels_;
};

class Texture2D {
public:
	Texture2D(const std::string& file_path);
	Texture2D(const Texture2DData& data);
	~Texture2D();

	void bind(unsigned int slot = 0)const;//纹理绑定的插槽,Windows系统通常有32个插槽
	void unbind()const;

	int width()const;
	int height()const;

private:
	static GLuint max_texture_units_;
	GLuint identity_;
	int width_, height_, channels_;//BPP每位像素的大小
};