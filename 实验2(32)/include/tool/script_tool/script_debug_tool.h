#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <expected>
#include <optional>

#include "tool/debug_tool.h"

//�Խű����ֵĴ�������޸ĵ���ʱʹ��
#define SCRIPT_DEBUG

#ifdef  SCRIPT_DEBUG
//��֤�ں���SCRIPT_DEBUGʱ��������д���λ��
#ifndef SCRIPT_SHOW_ERROR_LOCATION
//�����Ƿ���ʾ�ڳ����д�������λ��
#define SCRIPT_SHOW_ERROR_LOCATION
#endif // !SCRIPT_SHOW_ERROR_LOCATION

#endif //  SCRIPT_DEBUG

#define SCRIPT_COUT \
std::cout

#define SCRIPT_CLOG \
std::clog<<"[Script Log]:"

//������ļ���title��Ϣ���������
#define SCRIPT_COMPILE_ERROR_ADDITIONAL \
std::clog

//�����Ƿ���ʾ�ڳ����д�������λ��
#ifdef SCRIPT_SHOW_ERROR_LOCATION

#define SCRIPT_CERR \
std::cerr<<"[Script Error]"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"

#define SCRIPT_MODULE_INSERT_ERROR(module_set,module_name)\
std::clog<<"[Script Error](module)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
"(module set name:"<<module_set<<",module name:"<<module_name<<"):\n"\

#else

#define SCRIPT_CERR \
std::cerr<<"[Script Error]"

#endif // SHOW_ERROR_LOCATION

void print_header(std::ostream& os, const std::string& header);

void print_footer(std::ostream& os, const std::string& footer);