#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <expected>
#include <optional>

#include "tool/debug_tool.h"

//对脚本部分的代码进行修改调试时使用
#define SCRIPT_DEBUG

#ifdef  SCRIPT_DEBUG
//保证在含有SCRIPT_DEBUG时输出程序中错误位置
#ifndef SCRIPT_SHOW_ERROR_LOCATION
//控制是否显示在程序中错误发生的位置
#define SCRIPT_SHOW_ERROR_LOCATION
#endif // !SCRIPT_SHOW_ERROR_LOCATION

#endif //  SCRIPT_DEBUG

#define SCRIPT_COUT \
std::cout

#define SCRIPT_CLOG \
std::clog<<"[Script Log]:"

//在输出文件等title信息后输出内容
#define SCRIPT_COMPILE_ERROR_ADDITIONAL \
std::clog

//控制是否显示在程序中错误发生的位置
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