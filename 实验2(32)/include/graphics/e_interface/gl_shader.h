#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "graphic_tool.h"

class Shader {
	friend class UniformBuffer;
public:
	Shader(const std::string& file_path);
	~Shader();

	void bind() const;
	void unbind() const;
	GLuint identity()const;

	//Set uniforms
	void set_uniform1i(const std::string& name, int value);
	void set_uniform_arrayi(const std::string& name, const std::vector<int>& data);
	void set_uniform1f(const std::string& name, float value);
	void set_uniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void set_uniform_mat4f(const std::string& name, mat4 matrix);
	void set_uniform_array_mat4f(const std::string& name, const std::vector<mat4>& data);

private:
	struct ShaderProgramSource {
		std::string vertex_source, fragment_source;
	};

	ShaderProgramSource parse_shader(const std::string file_path);
	GLuint compile_shader(unsigned int type, const std::string& source);
	GLuint create_shader(const std::string& vertex_shader, const std::string& fragment_shader);
	int get_uniform_location(const std::string& name);

	std::string file_path_;
	GLuint identity_;
	std::unordered_map<std::string, int> uniform_location_cache_;
	//caching for uniforms
};

class UniformBuffer {
public:
	UniformBuffer(const std::string& block_name, const Shader& shader);
	~UniformBuffer();

	void set_uniform1i(const std::string& name, int value);
	void set_uniform1f(const std::string& name, float value);
	void set_uniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void set_uniform_mat4f(const std::string& name, mat4 matrix);

	void bind()const;
	void unbind()const;
	void update();
	unsigned int binding_point()const;
private:
	unsigned char* get_uniform_information(const std::string& name, GLuint& size, GLuint& type);

	struct UniformInfo {
		GLuint size, offset, type;
	};

	static GLuint number_of_uniform_block_;
	static const GLuint name_max_length_;
	bool update_flag_;
	GLuint identity_, block_size_, binding_point_;
	std::unordered_map<std::string, UniformInfo> index_;
	std::vector<unsigned char> local_buffer_;
	//std::vector<unsigned char>ø…“‘ÃÊªªvoid*
};