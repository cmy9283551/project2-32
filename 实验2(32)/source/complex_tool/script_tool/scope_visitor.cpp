#include "complex_tool/script_tool/scope_visitor.h"

#include <set>

void ScopeVisitor::ScopeNotFound::operator+=(const ScopeNotFound& that) {
	std::size_t size = that.variable_scope.size();
	for (std::size_t i = 0; i < size; i++) {
		variable_scope.emplace_back(that.variable_scope[i]);
	}
	size = that.function_scope.size();
	for (std::size_t i = 0; i < size; i++) {
		function_scope.emplace_back(that.function_scope[i]);
	}
}

ScopeVisitor::ScopeVisitor(
	const std::vector<std::pair<std::string, const VariableManager*>>& vm_ptr_list,
	const std::vector<std::pair<std::string, const FunctionManager*>>& fm_ptr_list
) :vm_ptr_container_(vm_ptr_list), fm_ptr_container_(fm_ptr_list) {
	check_name_space();
}

std::optional<ScopeVisitor::ScopeNotFound> ScopeVisitor::init_sub_scope(
	const std::vector<std::string>& vm_vector,
	const std::vector<std::string>& fm_vector,
	ScopeVisitor& sub_scope
) const {
	bool success = true;
	ScopeNotFound message;
	std::size_t size = vm_vector.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = vm_ptr_container_.find(vm_vector[i]);
		if (iter == vm_ptr_container_.cend()) {
			success = false;
			message.variable_scope.emplace_back(vm_vector[i]);
			continue;
		}
		sub_scope.vm_ptr_container_.insert(iter.first(), iter.second());
	}

	size = fm_vector.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = fm_ptr_container_.find(fm_vector[i]);
		if (iter == fm_ptr_container_.cend()) {
			success = false;
			message.function_scope.emplace_back(fm_vector[i]);
			continue;
		}
		sub_scope.fm_ptr_container_.insert(iter.first(), iter.second());
	}

	if (success == true) {
		return std::nullopt;
	}
	return message;
}

std::optional<ScopeVisitor::ScopeNotFound> ScopeVisitor::init_sub_scope(
	const ScopeVector& scope_vector, ScopeVisitor& sub_scope
) const {
	return init_sub_scope(scope_vector.variable_scope, scope_vector.function_scope, sub_scope);
}

void ScopeVisitor::get_scope_vector(
	std::vector<std::string>& vm_vector, std::vector<std::string>& fm_vector
) const {
	//保证有序,因为顺序会决定覆盖的先后
	auto v_visitors = vm_ptr_container_.get_visitor();
	std::size_t size = v_visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		vm_vector.emplace_back(*v_visitors[i].first);
	}

	auto f_visitors = fm_ptr_container_.get_visitor();
	size = f_visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		fm_vector.emplace_back(*f_visitors[i].first);
	}
}

void ScopeVisitor::get_scope_vector(ScopeVector& scope_vector) const {
	get_scope_vector(scope_vector.variable_scope, scope_vector.function_scope);
}

std::optional<const VariableManager*> ScopeVisitor::find_vm(
	const std::string& name
) const {
	auto iter = vm_ptr_container_.find(name);
	if (iter == vm_ptr_container_.cend()) {
		return std::nullopt;
	}
	return iter.second();
}

std::optional<const FunctionManager*> ScopeVisitor::find_fm(
	const std::string& name
) const {
	auto iter = fm_ptr_container_.find(name);
	if (iter == fm_ptr_container_.cend()) {
		return std::nullopt;
	}
	return iter.second();
}

std::expected<ScopeVisitor::IdentifierType, std::string> ScopeVisitor::identify(
	const std::string& name
) const{
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		auto is_var = vm_ptr_container_[i]->find(name);
		if (is_var != std::nullopt) {
			return IdentifierType::Variable;
		}
		auto is_type = vm_ptr_container_[i]->find_type(name);
		if (is_type != std::nullopt) {
			return IdentifierType::TypeName;
		}
	}
	size = fm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		auto is_func = fm_ptr_container_[i]->have(name);
		if (is_func == true) {
			return IdentifierType::Function;
		}
	}
	return IdentifierType::Null;
}

void ScopeVisitor::check_name_space(){
	//调试时,名称冲突会直接报错
	std::vector<std::string> name_vector;
	std::set<std::string> name_set;
#ifdef SCRIPT_DEBUG
	auto check = [&]() ->bool
		{
			std::size_t size = name_vector.size();
			for (std::size_t i = 0; i < size; i++) {
				if (name_set.contains(name_vector[i]) == true) {
					ASSERT(false);
					return false;
				}
				name_set.emplace(name_vector[i]);
			}
			name_vector.clear();
			return true;
		};
#else
	auto check = [&]() ->bool
		{
			std::size_t size = name_vector.size();
			for (std::size_t i = 0; i < size; i++) {
				if (name_set.contains(name_vector[i]) == true) {
					return false;
				}
				name_set.emplace(name_vector[i]);
			}
			name_vector.clear();
			return true;
		};
#endif 
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		vm_ptr_container_[i]->get_name_vector(name_vector);
		if (check() == false) {
			vm_ptr_container_.clear();
			fm_ptr_container_.clear();
			return;
		}
	}
	size = fm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		fm_ptr_container_[i]->get_name_vector(name_vector);
		if (check() == false) {
			vm_ptr_container_.clear();
			fm_ptr_container_.clear();
			return;
		}
	}
}
