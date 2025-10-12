#include "complex_tool/script_tool/SES_implementation/ses_module.h"

#include "complex_tool/script_tool/SES_implementation/ses_parser.h"

namespace ses {

	Module::Module(
		const std::string& name,
		std::unique_ptr<StructTemplateContainer> struct_template_container,
		IndexedMap<std::string, Function>& function_container,
		std::unique_ptr<ModuleConfig> module_config
	) :name_(name),
		struct_template_container_(std::move(struct_template_container)),
		function_container_(std::move(function_container)),
		module_config_(std::move(module_config))
	{
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

	std::optional<Module::StructProxy> Module::find_type(
		const std::string& identifier
	) const {
		auto result = struct_template_container_->find(identifier);
		if (result.has_value() == false) {
			return std::nullopt;
		}
		return StructProxy(result.value(), *struct_template_container_);
	}

	std::optional<Module::IdentifierType> Module::identify(
		const std::string& identifier
	) const {
		if (find_function(identifier).has_value() == true) {
			return IdentifierType::Function;
		}
		if (find_type(identifier).has_value() == true) {
			return IdentifierType::TypeName;
		}
		return std::nullopt;
	}

	std::optional<Module::ScopeNotFound> Module::check_scope(
		const ScopeVisitor& scope
	) const {
		ScopeNotFound message;
		ScopeVector scope_list;
		bool success = true;
		module_config_->scope_visitor.get_scope_vector(scope_list);

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

	std::optional<std::vector<std::string>> ModuleManager::init_sub_visitor(
		std::vector<std::string>& init_vector, ModuleVisitor& sub_visitor
	) const {
		bool success = true;
		std::vector<std::string> message;
		std::size_t size = init_vector.size();
		for (std::size_t i = 0; i < size; i++) {
			auto iter = modules_.find(init_vector[i]);
			if (iter == modules_.cend()) {
				success = false;
				message.emplace_back(init_vector[i]);
				continue;
			}
			sub_visitor.modules_.insert(iter.first(), &iter.second());
		}
		if (success == true) {
			return std::nullopt;
		}
		return message;
	}

	const std::string& ModuleManager::name()const {
		return name_;
	}

	bool ModuleManager::insert_module(
		const std::string& module_name,
		std::unique_ptr<StructTemplateContainer> struct_template_container,
		IndexedMap<std::string, Function>& function_container,
		std::unique_ptr<ModuleConfig> module_config
	) {
		auto iter = modules_.find(module_name);
		const auto& function_name = function_container.get_visitor();
		std::size_t size = function_name.size();
		for (std::size_t i = 0; i < size; i++) {
			if (name_space_.contains(*function_name[i].first) == true) {
				//由于module为脚本内容,因此只能通过日志输出错误,不允许抛出异常
				SCRIPT_MODULE_INSERT_ERROR(name_, module_name)
					<< "模组[" << module_name << "]中函数名称["
					<< *function_name[i].first << "]与已有名称冲突\n";
				return false;
			}
			name_space_.emplace(*function_name[i].first);
		}

		const auto& struct_name = struct_template_container->all_types().get_visitor();
		size = struct_name.size();
		for (std::size_t i = 0; i < size; i++) {
			if (name_space_.contains(*struct_name[i].first) == true) {
				//由于module为脚本内容,因此只能通过日志输出错误,不允许抛出异常
				SCRIPT_MODULE_INSERT_ERROR(name_, module_name)
					<< "模组[" << module_name << "]中类型名称["
					<< *struct_name[i].first << "]与已有名称冲突\n";
				return false;
			}
			name_space_.emplace(*struct_name[i].first);
		}

		modules_.emplace(
			module_name,
			Module(
				module_name,
				std::move(struct_template_container),
				function_container,
				std::move(module_config)
			)
		);
		return true;
	}

	void ModuleVisitor::get_module_vector(std::vector<std::string>& module_list)const {
		auto visitors = modules_.get_visitor();
		std::size_t size = visitors.size();
		for (std::size_t i = 0; i < size; i++) {
			module_list.emplace_back(*visitors[i].first);
		}
	}

	std::optional<std::pair<ModuleVisitor::FunctionPtr, std::string>>ModuleVisitor::find_function(
		const std::string& identifier
	)const {
		std::size_t size = modules_.size(), index, pointer;
		std::string message;
		bool found = false;
		for (std::size_t i = 0; i < size; i++) {
			auto result = modules_[i]->find_function(identifier);
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

	std::optional<std::pair<ModuleVisitor::StructProxy, std::string>> ModuleVisitor::find_type(
		const std::string& identifier
	)const {
		std::size_t size = modules_.size(), index;
		StructProxy proxy;
		std::string message;
		bool found = false;
		for (std::size_t i = 0; i < size; i++) {
			auto result = modules_[i]->find_type(identifier);
			if (result.has_value() == true) {
				if (found == true) {
					message = "存在多个名称为[" + identifier + "]的类型";
				}
				found = true;
				index = i;
				proxy = result.value();
			}
		}
		if (found == false) {
			return std::nullopt;
		}
		return { { proxy,message } };
	}

	std::optional<std::vector<std::string>> ModuleVisitor::init_sub_visitor(
		std::vector<std::string>& init_vector, ModuleVisitor& sub_visitor
	) const {
		bool success = true;
		std::vector<std::string> message;
		std::size_t size = init_vector.size();
		for (std::size_t i = 0; i < size; i++) {
			auto iter = modules_.find(init_vector[i]);
			if (iter == modules_.cend()) {
				success = false;
				message.emplace_back(init_vector[i]);
				continue;
			}
			sub_visitor.modules_.insert(iter.first(), iter.second());
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
		std::size_t size = modules_.size();
		for (std::size_t i = 0; i < size; i++) {
			auto result = modules_[i]->check_scope(scope);
			if (result != std::nullopt) {
				success = false;
				message.invalid_vector.emplace_back(
					modules_[i]->name(),
					result.value()
				);
			}
		}
		if (success == true) {
			return std::nullopt;
		}
		return message;
	}

	void ModuleVisitor::remove(const std::vector<std::string>& remove_vector) {
		std::size_t size = remove_vector.size();
		for (std::size_t i = 0; i < size; i++) {
			modules_.erase(remove_vector[i]);
		}
	}

	std::optional<ModuleVisitor::IdentifierType> ModuleVisitor::identify(
		const std::string& identifier
	)const {
		std::size_t size = modules_.size();
		for (std::size_t i = 0; i < size; i++) {
			auto result = modules_[i]->identify(identifier);
			if (result != std::nullopt) {
				return result;
			}
		}
		return std::nullopt;
	}
}