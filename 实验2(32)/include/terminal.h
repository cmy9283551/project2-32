#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "tool/script_tool/script_data.h"

class Terminal {
public:
	ScriptPackage calling(const std::string& func_name, const ScriptPackage& package);
private:
};