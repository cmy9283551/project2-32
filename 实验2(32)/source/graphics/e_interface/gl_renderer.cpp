#include "graphics/e_interface/gl_renderer.h"

#include <iostream>
#include <cstring>

#include "graphics/e_interface/graphic_tool.h"

Renderer::Renderer() {
	int max_texture_units = 0;
	GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units));
	max_texture_units_ = static_cast<GLuint>(max_texture_units);
}

void Renderer::enable_blend() const {
	GLCall(glEnable(GL_BLEND));
	//启用混合(可使多种在同一区域的颜色混合在一起)
	//glEnable(GL_BLEND) <-> glDisable(GL_BLEND)
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	//glBlendFunc(src,dest)
	//src:加入颜色的RGBA因子计算方式(乘数),默认GL_ONE
	//dest:原颜色的RGBA因子计算方式(乘数),默认GL_ZERO
	//GL_SRC_ALPHA : 原颜色透明度
	//GL_ONE_MINUS_SRC_ALPHA : 1减去src的颜色透明度
}

void Renderer::disable_blend() const {
	GLCall(glDisable(GL_BLEND));
}

void Renderer::clear(float red, float green, float blue, float alpha)const {
	GLCall(glClearColor(red, green, blue, alpha));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)const {
	shader.bind();
	va.bind();
	ib.bind();
	GLCall(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));
	//此处DrawElemnts是直接从GL_ELEMENT_ARRAY_BUFFER中读取
	//如果不用引索要用glDrawArrays,从VertexArray中读取,指定起点和数目
}

GLuint Renderer::max_texture_units() const {
	return max_texture_units_;
}

GLuint Renderer::available_texture_units() const {
	//这里暂时返回最大值
	return max_texture_units_;
}