#include "complex_tool/script_tool/function_manager.h"

#include "tool/script_tool/script_debug_tool.h"

FunctionManager::FunctionPtr::FunctionPtr(
	std::size_t index, FunctionManager& function_manager
) :pointer_(index), function_manager_(&function_manager) {
}

ScriptPackage FunctionManager::FunctionPtr::call(const ScriptPackage& data) {
	return function_manager_->call(pointer_,data);
}

std::optional<FunctionManager::FunctionPtr> BasicFunctionManager::find(
	const std::string& name
) {
	auto result = function_container_.find_serial_number(name);
	if (result.second == false) {
		return std::nullopt;
	}
	return { {result.first,*this} };
}

bool BasicFunctionManager::have(const std::string& name) const{
	auto iter = function_container_.find(name);
	if (iter == function_container_.cend()) {
		return false;
	}
	return true;
}

void BasicFunctionManager::get_name_vector(std::vector<std::string>& name_vector) const{
	auto iter = function_container_.cbegin();
	for (; iter != function_container_.cend(); ++iter) {
		name_vector.emplace_back(iter.first());
	}
}

ScriptPackage BasicFunctionManager::call(std::size_t index, const ScriptPackage& data){
	return function_container_[index](data);
}
