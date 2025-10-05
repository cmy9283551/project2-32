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
	//���û��(��ʹ������ͬһ�������ɫ�����һ��)
	//glEnable(GL_BLEND) <-> glDisable(GL_BLEND)
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	//glBlendFunc(src,dest)
	//src:������ɫ��RGBA���Ӽ��㷽ʽ(����),Ĭ��GL_ONE
	//dest:ԭ��ɫ��RGBA���Ӽ��㷽ʽ(����),Ĭ��GL_ZERO
	//GL_SRC_ALPHA : ԭ��ɫ͸����
	//GL_ONE_MINUS_SRC_ALPHA : 1��ȥsrc����ɫ͸����
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
	//�˴�DrawElemnts��ֱ�Ӵ�GL_ELEMENT_ARRAY_BUFFER�ж�ȡ
	//�����������Ҫ��glDrawArrays,��VertexArray�ж�ȡ,ָ��������Ŀ
}

GLuint Renderer::max_texture_units() const {
	return max_texture_units_;
}

GLuint Renderer::available_texture_units() const {
	//������ʱ�������ֵ
	return max_texture_units_;
}