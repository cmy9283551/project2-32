#include "complex_tool/script_tool/SES_implement/ses_module.h"

#include "complex_tool/script_tool/SES_implement/ses_parser.h"

namespace ses {

	Module::Module(
		const std::string& name,
		const std::string& struct_data,
		IndexedMap<std::string, Function>& function_container
	) :name_(name), struct_template_container_(struct_data),
		function_container_(std::move(function_container)) {
	}

	const std::string& Module::name()const {
		return name_;
	}

	std::optional<std::size_t> Module::find_function(
		const std::string& identifier
	)const {
		auto iter = function_container_.find(identifier);
		if (iter == function_container_.cend()) {
			return std::nullopt;
		}
		return iter.position();
	}

	std::optional<std::size_t> Module::find_type(
		const std::string& identifier
	) const {
		return struct_template_container_.find(identifier);
	}

	std::optional<Module::ScopeNotFound> Module::check_scope(
		const ScopeVisitor& scope
	) const {
		ScopeNotFound message;
		ScopeList scope_list;
		bool success = true;
		module_config_->scope_visitor.get_scope_list(scope_list);

		std::size_t size = scope_list.variable_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			if (scope.find_vm(scope_list.variable_scope[i]) == std::nullopt) {
				success = false;
				message.variable_scope.push_back(scope_list.variable_scope[i]);
			}
		}

		size = scope_list.function_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			if (scope.find_fm(scope_list.function_scope[i]) == std::nullopt) {
				success = false;
				message.function_scope.push_back(scope_list.function_scope[i]);
			}
		}

		if (success == true) {
			return std::nullopt;
		}
		return message;
	}

	ModuleManager::FunctionPtr::FunctionPtr(std::size_t module_index, std::size_t pointer)
		:module_index_(module_index), pointer_(pointer) {
	}

	ModuleManager::TypePtr::TypePtr(std::size_t module_index, std::size_t pointer)
		:module_index_(module_index), pointer_(pointer) {
	}

	std::optional<std::vector<std::string>> ModuleManager::init_sub_visitor(
		std::vector<std::string>& init_list, ModuleVisitor& sub_visitor
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

	std::optional<std::pair<ModuleVisitor::FunctionPtr, std::string>>ModuleVisitor::find_function(
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

	std::optional<std::pair<ModuleVisitor::TypePtr, std::string>> ModuleVisitor::find_type(
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

	std::optional<std::vector<std::string>> ModuleVisitor::init_sub_visitor(
		std::vector<std::string>& init_list, ModuleVisitor& sub_visitor
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
			sub_visitor.container_.emplace(iter.first(), iter.second());
		}
		if (success == true) {
			return std::nullopt;
		}
		return message;
	}

	std::optional<ModuleVisitor::InvalidModule> ModuleVisitor::check_scope(
		const ScopeVisitor& scope
	)const {
		InvalidModule message;
		bool success = true;
		std::size_t size = container_.size();
		for (std::size_t i = 0; i < size; i++) {
			auto result = container_[i]->check_scope(scope);
			if (result != std::nullopt) {
				success = false;
				message.invalid_list.emplace_back(
					container_[i]->name(),
					result.value()
				);
			}
		}
		if (success == true) {
			return std::nullopt;
		}
		return message;
	}

	void ModuleVisitor::remove(const std::vector<std::string>& remove_list) {
		std::size_t size = remove_list.size();
		for (std::size_t i = 0; i < size; i++) {
			container_.erase(remove_list[i]);
		}
	}
}