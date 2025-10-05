#pragma once

#include <memory>

#include "gl_vertex.h"
#include "gl_shader.h"

class Renderer {
public:
	Renderer();
	virtual ~Renderer() = default;

	void enable_blend()const;
	void disable_blend()const;

	void clear(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f)const;
	void draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)const;

	GLuint max_texture_units()const;
	GLuint available_texture_units()const;
private:
	GLuint max_texture_units_;
};
