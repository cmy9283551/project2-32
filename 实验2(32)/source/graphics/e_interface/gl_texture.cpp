#include "graphics/e_interface/gl_texture.h"

#include "graphics/e_interface/graphic_tool.h"

Texture2DData::Texture2DData(const std::string& file_path)
	:width_(0), height_(0), channels_(0) {
	set_flip_vertically_on_load(true);//翻转纹理,OpenGL默认(0,0)为左下角
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
	//按file_path读取图像并产生Texture

	set_flip_vertically_on_load(true);//翻转纹理,OpenGL默认(0,0)为左下角
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

	//Parameter:参数,下列都是对GL_TEXTURE_2D中的纹理设置参数
	//此处参数往往都填这几个,但不能不填
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//设置缩放参数,放大放小的采样原则
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_S:水平环绕,可看成width
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_T:竖直环绕,可看成hight
	//嵌入(环绕)模式
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, channel, width_, height_, 0, channel, GL_UNSIGNED_BYTE, local_buffer));
	//void glTexImage2D(	
	//  GLenum target,  /将数据写入哪个插槽所对应的texture对象
	//  GLint level,  /复合纹理,填0,不管
	//	GLint internalFormat, /填GL_RGBA8,不管
	//	GLsizei width,GLsizei height, /图像宽高
	//	GLint border, /填0,不管
	//	GLenum format, /填GL_RGBA,不管
	//	GLenum type, /数据类型
	//	const GLvoid* data /数据指针
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

	//Parameter:参数,下列都是对GL_TEXTURE_2D中的纹理设置参数
	//此处参数往往都填这几个,但不能不填
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//设置缩放参数,放大放小的采样原则
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_S:水平环绕,可看成width
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	//GL_TEXTURE_WRAP_T:竖直环绕,可看成hight
	//嵌入(环绕)模式
	GLCall(
		glTexImage2D
		(GL_TEXTURE_2D, 0, channel, width_, height_, 0, channel, GL_UNSIGNED_BYTE, data.data().data())
	);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture2D::~Texture2D() {
	//删除内存和显存中的纹理数据
	GLCall(glDeleteTextures(1, &identity_));
	identity_ = 0;
}

void Texture2D::bind(unsigned int slot) const {
	if (slot >= max_texture_units_) {
		ASSERT(false);
	}
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	//texture插槽不唯一,激活一个插槽,glBindTexture会将该texture绑定到激活的插槽中
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

