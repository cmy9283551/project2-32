#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

//glew要在glfw前包含

#include "tool/debugtool.h"

#include "glm_interface.h"
#include "stbi_interface.h"

#define GRAPHIC_DEBUG

const unsigned int MainWindowWidth = 1200;
const unsigned int MainWindowHeight = 800;
const std::string MainWindowName("main");

#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x,__FILE__,__LINE__))

#define GRAPHIC_CERR std::cerr<<"[Graphic Error]:"

#define GRAPHIC_CLOG std::clog<<"[Graphic log]:"

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

unsigned int GLGetTypeSize(unsigned int type);

std::string GLGetTypeName(unsigned int type);

enum class GraphicDataEnum;

GLuint GLTypeTransform(GraphicDataEnum type);

GraphicDataEnum GLTypeTransformInv(GLenum type);