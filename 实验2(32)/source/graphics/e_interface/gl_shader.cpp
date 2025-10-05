#include "graphics/e_interface/gl_shader.h"

#include <fstream> 
#include <sstream>
#include <cstring>

Shader::Shader(const std::string& file_path)
	:file_path_(file_path), identity_(0) {
	ShaderProgramSource source = parse_shader(file_path);
	identity_ = create_shader(source.vertex_source, source.fragment_source);
}

Shader::~Shader() {
	GLCall(glDeleteProgram(identity_));
}

void Shader::bind() const {
	GLCall(glUseProgram(identity_));
}

void Shader::unbind() const {
	GLCall(glUseProgram(0));
}

GLuint Shader::identity() const{
	return identity_;
}

void Shader::set_uniform1i(const std::string& name, int value) {
	bind();
	GLCall(glUniform1i(get_uniform_location(name), value));
}

void Shader::set_uniform_arrayi(const std::string& name, const std::vector<int>& data) {
	bind();
	GLCall(glUniform1iv(get_uniform_location(name), data.size(), data.data()));
}

void Shader::set_uniform1f(const std::string& name, float value) {
	bind();
	GLCall(glUniform1f(get_uniform_location(name), value));
}

void Shader::set_uniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	bind();
	GLCall(glUniform4f(get_uniform_location(name), v0, v1, v2, v3));
}

void Shader::set_uniform_mat4f(const std::string& name, mat4 matrix) {
	bind();
	GLCall(glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
	//glm�������ķ�ʽ��openglҪ��ķ�ʽ��ͬ,����Ҫת�þ���
}

void Shader::set_uniform_array_mat4f(const std::string& name, const std::vector<mat4>& data) {
	bind();
	GLCall(glUniformMatrix4fv(get_uniform_location(name), data.size(), GL_FALSE, &data[0][0][0]));
}

Shader::ShaderProgramSource Shader::parse_shader(const std::string file_path) {
	std::ifstream stream(file_path);

	if (stream.is_open() == false) {
		GRAPHIC_CERR <<
			"Fail to open shader file '" << file_path << "'" << std::endl;
		ASSERT(false);
	}

	std::cout << "Parsing shader file '" << file_path << "'" << std::endl;

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line)) {
		std::cout << line << std::endl;
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			switch (type) {
			case ShaderType::NONE:
				break;
			case ShaderType::VERTEX:
				ss[0] << line << '\n';
				break;
			case ShaderType::FRAGMENT:
				ss[1] << line << '\n';
				break;
			default:
				break;
			}
		}
	}

	return { ss[0].str(),ss[1].str() };
}

unsigned int Shader::compile_shader(unsigned int type, const std::string& source) {
	GLCall(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE) {
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = new char[length];
		GLCall(glGetShaderInfoLog(id, length, &length, message));
		GRAPHIC_CERR << "Fail to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
		GRAPHIC_CERR << message << std::endl;
		delete[] message;
		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

unsigned int Shader::create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
	GLCall(unsigned int program = glCreateProgram());
	unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
	unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	//�����ӵ���ɫ�����ӵ�����
	GLCall(glValidateProgram(program));
	//��֤�����Ƿ���ڵ�ǰ��������

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}


int Shader::get_uniform_location(const std::string& name) {
	if (uniform_location_cache_.find(name) != uniform_location_cache_.end()) {
		return uniform_location_cache_[name];
	}
	GLCall(int location = glGetUniformLocation(identity_, name.c_str()));
	if (location == -1) {
		GRAPHIC_CERR <<
			"uniform '" << name << "' doesn't exist"
			<< std::endl;
		ASSERT(false);
	}
	else {
		uniform_location_cache_[name] = location;
	}
	return location;
}

unsigned int UniformBuffer::number_of_uniform_block_ = 0;
const unsigned int UniformBuffer::name_max_length_ = 256;

UniformBuffer::UniformBuffer(const std::string& block_name, const Shader& shader)
	:identity_(0), update_flag_(false) {
	unsigned int program = shader.identity_;

	//ȷ����ɫ����
	shader.bind();

	//���UBO��С
	GLCall(int index = glGetUniformBlockIndex(program, block_name.c_str()));
	if (index == -1) {
		GRAPHIC_CERR <<
			"uniform block '" << block_name << "' doesn't exist"
			<< std::endl;
		ASSERT(false);
	}
	int size, block_size;
	GLCall(glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size));
	GLCall(glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &size));
	block_size_ = block_size;

	//����������
	GLCall(glCreateBuffers(1, &identity_));
	//glCreateBuffers�ᴴ������������,������Ϊ����䴢��ռ�
	//��ͨ��glNamedBufferData/glNamedBufferStorage���䴢��ռ�
	GLCall(glNamedBufferStorage(identity_, block_size_, NULL, GL_MAP_WRITE_BIT));

	//buffer��uniform���
	GLCall(glUniformBlockBinding(program, index, number_of_uniform_block_));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, number_of_uniform_block_, identity_));
	binding_point_ = number_of_uniform_block_;
	number_of_uniform_block_++;

	//��ʼ������
	local_buffer_.resize(block_size_);
	std::vector<int> indices(size);

	//��ȡuniform block��Ϣ
	GLCall(glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data()));
	char name[name_max_length_];
	int offset, name_lenght, count;
	unsigned int type;
	for (unsigned int idx : indices) {
		GLCall(glGetActiveUniform(program, idx, name_max_length_, &name_lenght, &count, &type, name));
		GLCall(glGetActiveUniformsiv(program, 1, &idx, GL_UNIFORM_OFFSET, &offset));
		index_[name] = { unsigned int(count) * GLGetTypeSize(type),unsigned int(offset) ,type };
	}
}

UniformBuffer::~UniformBuffer() {
	GLCall(glDeleteBuffers(1, &identity_));
}

#define CHECK_TYPE(GLenum)\
	update_flag_ = true;\
	unsigned int size, type;\
	auto ptr = get_uniform_information(name, size, type);\
	if(type!=GLenum){\
		GRAPHIC_CERR <<\
		"The type of the uniform '" << name << "' is not " << GLGetTypeName(GLenum) <<\
		",it's " << GLGetTypeName(type)\
		<< std::endl;\
		ASSERT(false);\
	}

void UniformBuffer::set_uniform1i(const std::string& name, int value) {
	CHECK_TYPE(GL_INT);
	std::memcpy(ptr, &value, size);
}

void UniformBuffer::set_uniform1f(const std::string& name, float value) {
	CHECK_TYPE(GL_FLOAT);
	std::memcpy(ptr, &value, size);
}

void UniformBuffer::set_uniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	CHECK_TYPE(GL_FLOAT_VEC4);
	float data[4] = { v0,v1,v2,v3 };
	std::memcpy(ptr, &data[0], size);
}

void UniformBuffer::set_uniform_mat4f(const std::string& name, mat4 matrix) {
	CHECK_TYPE(GL_FLOAT_MAT4);
	std::memcpy(ptr, &matrix[0][0], size);
}

#undef CHECK_TYPE

void UniformBuffer::bind()const {
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, identity_));
}

void UniformBuffer::unbind()const {
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBuffer::update() {
	if (update_flag_ == false) {
		return;
	}
	bind();
	GLCall(void* buffer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
	memcpy(buffer, local_buffer_.data(), block_size_);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	//ʹ��glMapBuffer��ʼ�����������ϱ�glBufferSubData����
	update_flag_ = false;
}

unsigned int UniformBuffer::binding_point() const {
	return binding_point_;
}

unsigned char* UniformBuffer::get_uniform_information(const std::string& name, GLuint& size, GLuint& type) {
	//ȡ��uniform����������������,���������ڸ���(UniformBuffer)���ڴ��(local_buffer_)��λ��(ƫ����)
	auto iter = index_.find(name);
	if (iter == index_.end()) {
		GRAPHIC_CERR <<
			"uniform '" << name << "' doesn't exist"
			<< std::endl;
		ASSERT(false);
	}

	size = iter->second.size;
	type = iter->second.type;
	return local_buffer_.data() + iter->second.offset;
	//ANSI�涨���ܶ�void*�����������,��GUN�涨���Բ���,��+1��ʾ����һ�ֽ�
	//ANSI�涨�¿ɽ�void*ת��unsigned char*�ٽ��в���
	//��˿�����unsigned char*�滻void* 
}