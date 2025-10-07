#pragma once

#include <iostream>
#include <optional>

#define ASSERT(x) if(!(x)) __debugbreak();

#define TOOL_ERR std::cerr<<"[Tool Error]:"

#define ERROR(x) \
if(x){\
	std::cerr<<"[Error](file:"<<__FILE__<<")(line:"<<__LINE__<<")\n(func:"<<__func__<<")";\
}\
else{\
	std::cerr<<"[Error]";\
}\
std::cerr << ":"