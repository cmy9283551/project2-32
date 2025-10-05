#include "graphics/e_interface/graphic_tool.h"

#include "graphics/graphic_enum.h"

#include <iostream>

void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
	while (GLenum error = glGetError()) {
		GRAPHIC_CERR << "[OpenGL Error] (" << error << "):"
			<< function << " " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

unsigned int GLGetTypeSize(unsigned int type) {
	switch (type) {
	case GL_FLOAT:              return 1 * sizeof(GLfloat);
	case GL_FLOAT_VEC2:			return 2 * sizeof(GLfloat);
	case GL_FLOAT_VEC3:			return 3 * sizeof(GLfloat);
	case GL_FLOAT_VEC4:			return 4 * sizeof(GLfloat);
	case GL_INT:				return 1 * sizeof(GLint);
	case GL_INT_VEC2:			return 2 * sizeof(GLint);
	case GL_UNSIGNED_INT:		return 1 * sizeof(GLuint);
	case GL_UNSIGNED_INT_VEC2:  return 2 * sizeof(GLuint);
	case GL_BOOL:				return 1 * sizeof(GLboolean);
	case GL_BOOL_VEC2:			return 2 * sizeof(GLboolean);
	case GL_FLOAT_MAT2:			return 4 * sizeof(GLfloat);
	case GL_FLOAT_MAT2x3:		return 6 * sizeof(GLfloat);
	case GL_FLOAT_MAT4:			return 16 * sizeof(GLfloat);
	case GL_UNSIGNED_BYTE:		return 1 * sizeof(GLubyte);
	}
	ASSERT(false);
	return 0;
}

std::string GLGetTypeName(unsigned int type) {
	switch (type) {
	case GL_FLOAT:				return "float";
	case GL_FLOAT_VEC2:			return "vec2";
	case GL_FLOAT_VEC3:			return "vec3";
	case GL_FLOAT_VEC4:			return "vec4";
	case GL_INT:				return "int";
	case GL_INT_VEC2:			return "ivec2";
	case GL_INT_VEC3:			return "ivec3";
	case GL_INT_VEC4:			return "ivec4";
	case GL_BOOL:				return "bool";
	case GL_BOOL_VEC2:			return "bvec2";
	case GL_BOOL_VEC3:			return "bvec3";
	case GL_BOOL_VEC4:			return "bvec4";
	case GL_FLOAT_MAT2:			return "mat2";
	case GL_FLOAT_MAT3:			return "mat3";
	case GL_FLOAT_MAT4:			return "mat4";
	case GL_SAMPLER_2D:			return "sampler2D";
	default:					return "unknown";
	}
}

GLuint GLTypeTransform(GraphicDataEnum type){
	switch (type)
	{
	case GraphicDataEnum::Byte:
		return GL_BYTE;
	case GraphicDataEnum::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case GraphicDataEnum::Short:
		return GL_SHORT;
	case GraphicDataEnum::UnsignedShort:
		return GL_UNSIGNED_SHORT;
	case GraphicDataEnum::Int:
		return GL_INT;
	case GraphicDataEnum::UnsignedInt:
		return GL_UNSIGNED_INT;
	case GraphicDataEnum::Float:
		return GL_FLOAT;
	case GraphicDataEnum::Double:
		return GL_DOUBLE;
	default:
		ASSERT(false);
		break;
	}
	return 0;
}

GraphicDataEnum GLTypeTransformInv(GLenum type){
	switch (type)
	{
	case GL_BYTE:
		return GraphicDataEnum::Byte;
	case  GL_UNSIGNED_BYTE:
		return GraphicDataEnum::UnsignedByte;
	case GL_SHORT:
		return GraphicDataEnum::Short;
	case GL_UNSIGNED_SHORT:
		return GraphicDataEnum::UnsignedShort;
	case GL_INT:
		return GraphicDataEnum::Int;
	case GL_UNSIGNED_INT:
		return GraphicDataEnum::UnsignedInt;
	case GL_FLOAT:
		return GraphicDataEnum::Float;
	case GL_DOUBLE:
		return GraphicDataEnum::Double;
	default:
		ASSERT(false);
		break;
	}
	return GraphicDataEnum::Null;
}
