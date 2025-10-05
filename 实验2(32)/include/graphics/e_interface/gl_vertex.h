#pragma once

#include <vector>
#include <memory>

#include "graphic_tool.h"

class VertexBuffer {
public:
	VertexBuffer();
	VertexBuffer(GLuint size);
	VertexBuffer(GLuint size, const void* data);
	VertexBuffer(const std::vector<unsigned char>& data);
	~VertexBuffer();

	void bind()const;
	void unbind()const;
	void sub_data(GLuint offset, GLuint size, const void* data)const;
	void get_data(GLuint size, const void* data)const;
	void get_data(const std::vector<unsigned char>& data)const;
private:
	GLuint identity_;
};

class IndexBuffer {
public:
	IndexBuffer();
	IndexBuffer(GLuint count);
	IndexBuffer(GLuint count, GLuint* data);
	IndexBuffer(const std::vector<GLuint>& data);
	~IndexBuffer();

	void bind()const;
	void unbind()const;
	void sub_data(GLuint offset, GLuint count, const GLuint* data)const;
	void get_data(const std::vector<GLuint>& data);
	void get_data(GLuint count, GLuint* data);
	GLuint get_count()const;
private:
	GLuint identity_, count_;
};

struct VertexBufferElement {
	GLenum type;
	GLuint count;
	unsigned char normalized;

	GLuint size()const;
	GraphicDataEnum cms_type()const;
};

class VertexBufferLayout {
public:
	VertexBufferLayout();
	~VertexBufferLayout() = default;

	void push(GraphicDataEnum type, GLuint count, bool normalized = false);

	const std::vector<VertexBufferElement> get_elements() const;

	GLuint get_stride() const;
	GLuint number_of_elements() const;
	GLuint vertex_size()const;
private:
	std::vector<VertexBufferElement> elements_;
	GLuint stride_;
};


class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void bind_buffer(const VertexBuffer& vb, const VertexBufferLayout& layout)const;

	void bind() const;
	void unbind() const;
private:
	GLuint identity_;
};