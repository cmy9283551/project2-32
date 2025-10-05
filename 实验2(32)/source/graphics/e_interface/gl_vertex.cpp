#include "graphics/e_interface/gl_vertex.h"

#include "graphics/e_interface/graphic_tool.h"

VertexBuffer::VertexBuffer() {
	GLCall(glCreateBuffers(1, &identity_));
}

VertexBuffer::VertexBuffer(GLuint size) {
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, size, NULL, GL_DYNAMIC_DRAW));
}

VertexBuffer::VertexBuffer(GLuint size, const void* data) {
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, size, data, GL_DYNAMIC_DRAW));
}

VertexBuffer::VertexBuffer(const std::vector<unsigned char>& data) {
	//glGenBuffersֻ��������
	//���״�ͨ������glBindBuffer��֮ǰ,û���κλ����������뷵�صĻ������������������
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, data.size(), data.data(), GL_DYNAMIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
	GLCall(glDeleteBuffers(1, &identity_));
}

void VertexBuffer::bind() const {
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, identity_));
}

void VertexBuffer::unbind() const {
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::sub_data(GLuint offset, GLuint size, const void* data) const {
	GLCall(glNamedBufferSubData(identity_, offset, size, data));
}

void VertexBuffer::get_data(GLuint size, const void* data) const {
	GLCall(glNamedBufferData(identity_, size, data, GL_DYNAMIC_DRAW));
}

void VertexBuffer::get_data(const std::vector<unsigned char>& data)const {
	GLCall(glNamedBufferData(identity_, data.size(), data.data(), GL_DYNAMIC_DRAW));
}

IndexBuffer::IndexBuffer()
	:count_(0) {
	GLCall(glCreateBuffers(1, &identity_));
}

IndexBuffer::IndexBuffer(GLuint count)
	:count_(count) {
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, count_ * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW));
}

IndexBuffer::IndexBuffer(GLuint count, GLuint* data)
	:count_(count) {
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, count_ * sizeof(GLuint), data, GL_DYNAMIC_DRAW));
}

IndexBuffer::IndexBuffer(const std::vector<GLuint>& data)
	:count_(data.size()) {
	GLCall(glCreateBuffers(1, &identity_));
	GLCall(glNamedBufferData(identity_, count_ * sizeof(GLuint), data.data(), GL_DYNAMIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
	GLCall(glDeleteBuffers(1, &identity_));
}

void IndexBuffer::bind()const {
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, identity_));
}

void IndexBuffer::unbind() const {
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::sub_data(GLuint offset, GLuint count, const GLuint* data)const {
	GLCall(glNamedBufferSubData(identity_, offset * sizeof(GLuint), count * sizeof(GLuint), data));
}

void IndexBuffer::get_data(const std::vector<GLuint>& data) {
	count_ = data.size();
	GLCall(glNamedBufferData(identity_, count_ * sizeof(GLuint), data.data(), GL_DYNAMIC_DRAW));
}

void IndexBuffer::get_data(GLuint count, GLuint* data) {
	count_ = count;
	GLCall(glNamedBufferData(identity_, count_ * sizeof(GLuint), data, GL_DYNAMIC_DRAW));
}

GLuint IndexBuffer::get_count() const {
	return count_;
}

GLuint VertexBufferElement::size() const {
	return count * GLGetTypeSize(type);
}

GraphicDataEnum VertexBufferElement::cms_type() const{
	return GLTypeTransformInv(type);
}

VertexBufferLayout::VertexBufferLayout()
	:stride_(0) {
}

void VertexBufferLayout::push(GraphicDataEnum type, GLuint count, bool normalized) {
	if (count > 4) {
		ASSERT(false);
	}
	GLenum gl_type = GLTypeTransform(type);

#define CASE(Type)\
	case Type:elements_.push_back({Type,count,normalized});\
	stride_ += count * GLGetTypeSize(Type);\
	break	
	switch (gl_type) {
		CASE(GL_FLOAT);
		CASE(GL_INT);
		CASE(GL_UNSIGNED_INT);
		CASE(GL_UNSIGNED_BYTE);
	default:
		GRAPHIC_CERR << "Unknown type:[" << GLGetTypeName(gl_type) << "]" << std::endl;
		ASSERT(false);
		break;
	}
#undef CASE
}

const std::vector<VertexBufferElement> VertexBufferLayout::get_elements() const {
	return elements_;
}

GLuint VertexBufferLayout::get_stride() const {
	return stride_;
}

GLuint VertexBufferLayout::number_of_elements() const {
	return elements_.size();
}

GLuint VertexBufferLayout::vertex_size() const {
	return stride_;
}

VertexArray::VertexArray() {
	GLCall(glCreateVertexArrays(1, &identity_));
}

VertexArray::~VertexArray() {
	GLCall(glDeleteVertexArrays(1, &identity_));
}

void VertexArray::bind_buffer(const VertexBuffer& vb, const VertexBufferLayout& layout)const {
	//��GL_ARRAY_BUFFER�е����ݷָ��layout���涨�ĸ�ʽ
	//��󽫷ָ�����ݷָ�ɲ�ͬ��ŵ�VertexAttibArray
	//VertexArray��VertexBufferҪͬʱ����
	bind();
	vb.bind();
	const auto& elements = layout.get_elements();
	GLuint offset = 0;//���һ�����Ե�ƫ����
	for (GLuint i = 0; i < elements.size(); i++) {
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type,
			element.normalized, layout.get_stride(), (const void*)offset));
		//��ʽ
		//void glVertexAttribPointer(GLuint	index, /ĳ���Ա��
		//	GLint	size,
		//	GLenum	type,
		//	GLboolean	normalized, /charҪ��ʽ��
		//	GLsizei	stride, /����һ��������Ծ���
		//	const GLvoid * pointer /���һ�����Ե�ƫ����
		// );
		//�ڸú�����,GL_ARRAY_BUFFER������ת�浽VertexArray��
		//��
		//The buffer object binding (GL_ARRAY_BUFFER_BINDING) is saved as 
		//generic vertex attribute array state (GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING) for index
		//(docs.GL����)
		offset += element.count * GLGetTypeSize(element.type);
	}
}

void VertexArray::bind() const {
	GLCall(glBindVertexArray(identity_));
}

void VertexArray::unbind() const {
	GLCall(glBindVertexArray(0));
}