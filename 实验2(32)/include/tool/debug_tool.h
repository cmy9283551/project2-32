#pragma once

#include <iostream>
#include <optional>

#define ASSERT(x) if(!(x)) __debugbreak();

#define TO_DO_ASSERT \
std::cerr<<"[To Do](file:"\
<<__FILE__<<")(line:"<<__LINE__<<")\n(func:"<<__func__<<")"<<std::endl;\
ASSERT(false);

#define TOOL_ERR std::cerr<<"[Tool Error]:"

#define ERROR(x) \
if(x){\
	std::cerr<<"[Error](file:"<<__FILE__<<")(line:"<<__LINE__<<")\n(func:"<<__func__<<")";\
}\
else{\
	std::cerr<<"[Error]";\
}\
std::cerr << ":"