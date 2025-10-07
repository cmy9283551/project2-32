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

#define SCRIPT_PARSER_COMPILE_WARNING(script_file,script_name,token)\
std::clog<<"[Script Warning](parser)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#define SCRIPT_PARSER_COMPILE_ERROR(script_file,error_line,script_name,token)\
std::clog<<"[Script Warning](parser)"<<\
"(file:"<<__FILE__<<")\n(line:"<<error_line<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#else

#define SCRIPT_CERR \
std::cerr<<"[Script Error]"

#define SCRIPT_LEXER_COMPILE_ERROR(script_file)\
std::clog<<"[Script Error](lexer)"<<"(script file:"<<script_file<<"):\n"

#define SCRIPT_LEXER_COMPILE_WARNING(script_file)\
std::clog<<"[Script Warning](lexer)"<<"(script file:"<<script_file<<"):\n"

#endif // SHOW_ERROR_LOCATION