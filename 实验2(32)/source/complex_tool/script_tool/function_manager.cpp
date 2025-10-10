#include "complex_tool/script_tool/function_manager.h"

#include "tool/script_tool/script_debug_tool.h"
#include "complex_tool/script_tool/variable_manager.h"

const std::string& FunctionManager::name()const {
	return name_;
}

FunctionManager::FunctionPtr::FunctionPtr(
	std::size_t index, FunctionManager& function_manager
) :pointer_(index), function_manager_(&function_manager) {
}

ScriptPackage FunctionManager::FunctionPtr::call(const ScriptPackage& data) {
	return function_manager_->call(pointer_, data);
}

BasicFunctionManager::BasicFunctionManager(const std::string& name)
	:FunctionManager(name) {
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

FunctionManager::FunctionManager(const std::string& name)
	:name_(name) {
}

bool BasicFunctionManager::have(const std::string& name) const {
	auto iter = function_container_.find(name);
	if (iter == function_container_.cend()) {
		return false;
	}
	return true;
}

ScriptPackage BasicFunctionManager::call(std::size_t index, const ScriptPackage& data) {
	return function_container_[index](data);
}

bool BasicFunctionManager::has_name_conflict(const VariableManager& vm) const {
	return vm.has_name_conflict(*this);
}

bool BasicFunctionManager::has_name_conflict(const FunctionManager& fm) const {
	auto iter = function_container_.cbegin();
	for (; iter != function_container_.cend(); ++iter) {
		if (fm.have(iter.first()) == true) {
			return true;
		}
	}
	return false;
}