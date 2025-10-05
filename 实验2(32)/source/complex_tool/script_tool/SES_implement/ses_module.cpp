#include "complex_tool/script_tool/SES_implement/ses_module.h"

SESModule::SESModule(
	const std::string& struct_data,
	IndexedMap<std::string, SESFunction>& function_container
) :struct_template_container_(struct_data),
function_container_(std::move(function_container)) {
}

std::optional<std::size_t> SESModule::find_function(
	const std::string& identifier
)const {
	auto iter = function_container_.find(identifier);
	if (iter == function_container_.cend()) {
		return std::nullopt;
	}
	return iter.position();
}

std::optional<std::size_t> SESModule::find_type(
	const std::string& identifier
) const {
	return struct_template_container_.find(identifier);
}

SESModuleManager::FunctionPtr::FunctionPtr(std::size_t module_index, std::size_t pointer)
	:module_index_(module_index), pointer_(pointer) {
}

SESModuleManager::TypePtr::TypePtr(std::size_t module_index, std::size_t pointer)
	:module_index_(module_index), pointer_(pointer) {
}

std::optional<std::vector<std::string>> SESModuleManager::init_visitor(
	std::vector<std::string>& init_list, SESModuleVisitor& sub_visitor
) const {
	bool success = true;
	std::vector<std::string> message;
	std::size_t size = init_list.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = container_.find(init_list[i]);
		if (iter == container_.cend()) {
			success = false;
			message.push_back(init_list[i]);
		}
		sub_visitor.container_.emplace(iter.first(), &iter.second());
	}
	if (success == true) {
		return std::nullopt;
	}
	return message;
}

std::optional<std::pair<SESModuleVisitor::FunctionPtr, std::string>> SESModuleVisitor::find_function(
	const std::string& identifier
)const {
	std::size_t size = container_.size(), index, pointer;
	std::string message;
	bool found = false;
	for (std::size_t i = 0; i < size; i++) {
		auto result = container_[i]->find_function(identifier);
		if (result.has_value() == true) {
			if (found == true) {
				message = "存在多个名称为[" + identifier + "]的函数";
			}
			found = true;
			index = i;
			pointer = result.value();
		}
	}
	if (found == false) {
		return std::nullopt;
	}
	return { { {index,pointer},message } };
}

std::optional<std::pair<SESModuleVisitor::TypePtr, std::string>> SESModuleVisitor::find_type(
	const std::string& identifier
)const {
	std::size_t size = container_.size(), index, pointer;
	std::string message;
	bool found = false;
	for (std::size_t i = 0; i < size; i++) {
		auto result = container_[i]->find_type(identifier);
		if (result.has_value() == true) {
			if (found == true) {
				message = "存在多个名称为[" + identifier + "]的类型";
			}
			found = true;
			index = i;
			pointer = result.value();
		}
	}
	if (found == false) {
		return std::nullopt;
	}
	return { { {index,pointer},message } };
}