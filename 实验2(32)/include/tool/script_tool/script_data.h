#pragma once

#include <string>
#include <vector>
#include <memory>

#include "script_debug_tool.h"

typedef int ScriptInt;
typedef float ScriptFloat;
typedef char ScriptChar;
typedef std::string ScriptString;
typedef std::vector<ScriptInt> ScriptVectorInt;
typedef std::vector<ScriptFloat> ScriptVectorFloat;

class ScriptPackage {
public:
	enum class DataType {
		Int,
		UnsignedInt,
		Float,
		String,
		Arrayi,
		Arrayui,
		Arrayf,
		Package
	};
private:
	std::vector<std::pair<DataType, std::size_t>> pointer_;
	std::vector<unsigned char> data_;
};

