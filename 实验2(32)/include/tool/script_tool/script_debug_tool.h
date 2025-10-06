#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <expected>
#include <optional>

#include "tool/debugtool.h"

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
#define SCRIPT_COMPILE_ERROR \
std::clog

//�����Ƿ���ʾ�ڳ����д�������λ��
#ifdef SCRIPT_SHOW_ERROR_LOCATION

#define SCRIPT_CERR \
std::cerr<<"[Script Error]"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"

#define SCRIPT_LEXER_COMPILE_ERROR(script_file)\
std::clog<<"[Script Error](lexer)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"

#define SCRIPT_LEXER_COMPILE_WARNING(script_file)\
std::clog<<"[Script Warning](lexer)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"

#define SCRIPT_PARSER_COMPILE_ERROR(script_file,script_name,token)\
std::clog<<"[Script Error](parser)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#else

#define SCRIPT_ERROR(error)\
error.error_count++;\
error.message<<"[Script Error]:"

#define SCRIPT_LEXER_COMPILE_ERROR(script_file) \
std::clog<<"[Script Error](lexer)(script file:"<<script_file<<"):\n"

#endif // SHOW_ERROR_LOCATION

