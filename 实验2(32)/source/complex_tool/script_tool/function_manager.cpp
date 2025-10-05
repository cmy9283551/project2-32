#include "complex_tool/script_tool/function_manager.h"

#include "tool/script_tool/script_debug_tool.h"

FunctionManager::FunctionProxy::FunctionProxy(
	std::size_t index, FunctionManager& function_manager
) :index_(index), function_manager_(function_manager) {
}

ScriptPackage FunctionManager::FunctionProxy::call(const ScriptPackage& data) {
	return function_manager_.call(index_,data);
}

std::optional<FunctionManager::FunctionProxy> BasicFunctionManager::find(
	const std::string& name
) {
	auto result = container_.find_serial_number(name);
	if (result.second == false) {
		return std::nullopt;
	}
	return { {result.first,*this} };
}

ScriptPackage BasicFunctionManager::call(std::size_t index, const ScriptPackage& data){
	return container_[index](data);
}
