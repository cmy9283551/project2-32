#include "graphics/e_interface/gl_texture.h"

#include "graphics/e_interface/graphic_tool.h"

Texture2DData::Texture2DData(const std::string& file_path)
	:width_(0), height_(0), channels_(0) {
	set_flip_vertically_on_load(true);//��ת����,OpenGLĬ��(0,0)Ϊ���½�
	unsigned char* local_buffer = image_load(file_path.c_str(), &width_, &height_, &channels_, 0);

#ifdef GRAPHIC_DEBUG
	std::clog << file_path << std::endl;
#endif // GRAPHIC_DEBUG

	if (local_buffer == nullptr) {
		GRAPHIC_CERR <<
			"There is no texture under the path :" << file_path << std::endl;
		ASSERT(false);
	}

	std::size_t size = width_ * height_ * channels_;
	data_.assign(local_buffer, local_buffer + size);

	image_free(local_buffer);
}

Texture2DData::~Texture2DData(){

}

int Texture2DData::width() const{
	return width_;
}

int Texture2DData::height() const{
	return height_;
}

int Texture2DData::channels() const{
	return channels_;
}

const std::vector<unsigned char>& Texture2DData::data()const {
	return data_;
}

GLuint Texture2D::max_texture_units_ = 0;

Texture2D::Texture2D(const std::string& file_path)
	:identity_(0), width_(0), height_(0), channels_(0) {
	if (max_texture_units_ == 0) {
		int max_units;
		GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units));
		max_texture_units_ = max_units;
	}
	//��file_path��ȡͼ�񲢲���Texture

	set_flip_vertically_on_load(true);//��ת����,OpenGLĬ��(0,0)Ϊ���½�
	unsigned char* local_buffer = image_load(file_path.c_str(), &width_, &height_, &channels_, 0);

	GLenum channel;
	if (channels_ == 3) {
		channel = GL_RGB;
	}
	else if (channels_ == 4) {
		channel = GL_RGBA;
	}
	else {
		ASSERT(false);
	}

	GLCall(glGenTextures(1, &identity_));
	GLCall(glBindTexture(GL_TEXTURE_2D, identity_));

	//Parameter:����,���ж��Ƕ�GL_TEXTURE_2D�е��������ò���
	//�˴��������������⼸��,�����ܲ���
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�������Ų���,�Ŵ��С�Ĳ���ԭ��
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_S:ˮƽ����,�ɿ���width
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_T:��ֱ����,�ɿ���hight
	//Ƕ��(����)ģʽ
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, channel, width_, height_, 0, channel, GL_UNSIGNED_BYTE, local_buffer));
	//void glTexImage2D(	
	//  GLenum target,  /������д���ĸ��������Ӧ��texture����
	//  GLint level,  /��������,��0,����
	//	GLint internalFormat, /��GL_RGBA8,����
	//	GLsizei width,GLsizei height, /ͼ����
	//	GLint border, /��0,����
	//	GLenum format, /��GL_RGBA,����
	//	GLenum type, /��������
	//	const GLvoid* data /����ָ��
	// );
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	if (local_buffer == nullptr) {
		GRAPHIC_CERR <<
			"There is no texture under the path :" << file_path << std::endl;
		ASSERT(false);
	}

	image_free(local_buffer);
}

Texture2D::Texture2D(const Texture2DData& data)
	:identity_(0), width_(data.width()), height_(data.height()), channels_(data.channels()) {
	if (max_texture_units_ == 0) {
		int max_units;
		GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units));
		max_texture_units_ = max_units;
	}

	GLenum channel;
	if (channels_ == 3) {
		channel = GL_RGB;
	}
	else if (channels_ == 4) {
		channel = GL_RGBA;
	}
	else {
		ASSERT(false);
	}

	GLCall(glGenTextures(1, &identity_));
	GLCall(glBindTexture(GL_TEXTURE_2D, identity_));

	//Parameter:����,���ж��Ƕ�GL_TEXTURE_2D�е��������ò���
	//�˴��������������⼸��,�����ܲ���
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�������Ų���,�Ŵ��С�Ĳ���ԭ��
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_S:ˮƽ����,�ɿ���width
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_T:��ֱ����,�ɿ���hight
	//Ƕ��(����)ģʽ
	GLCall(
		glTexImage2D
		(GL_TEXTURE_2D, 0, channel, width_, height_, 0, channel, GL_UNSIGNED_BYTE, data.data().data())
	);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture2D::~Texture2D() {
	//ɾ���ڴ���Դ��е���������
	GLCall(glDeleteTextures(1, &identity_));
	identity_ = 0;
}

void Texture2D::bind(unsigned int slot) const {
	if (slot >= max_texture_units_) {
		ASSERT(false);
	}
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	//texture��۲�Ψһ,����һ�����,glBindTexture�Ὣ��texture�󶨵�����Ĳ����
	GLCall(glBindTexture(GL_TEXTURE_2D, identity_));
}

void Texture2D::unbind() const {
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

int Texture2D::width() const {
	return width_;
}

int Texture2D::height() const {
	return height_;
}

