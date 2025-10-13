#include "complex_tool/script_tool/scope_visitor.h"

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
	const std::vector<const VariableManager*>& vm_ptr_list,
	const std::vector<const FunctionManager*>& fm_ptr_list
) {
	std::size_t size = vm_ptr_list.size();
	for (std::size_t i = 0; i < size; i++) {
		if (is_effective_scope(vm_ptr_list[i]) == true) {
			vm_ptr_container_.insert(vm_ptr_list[i]->name(), vm_ptr_list[i]);
		}
		else {
			return;
		}
	}
	size = fm_ptr_list.size();
	for (std::size_t i = 0; i < size; i++) {
		if (is_effective_scope(fm_ptr_list[i]) == true) {
			fm_ptr_container_.insert(fm_ptr_list[i]->name(), fm_ptr_list[i]);
		}
		else {
			return;
		}
	}
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

bool ScopeVisitor::insert_vm(const VariableManager* vm_ptr) {
	if (is_effective_scope(vm_ptr) == true) {
		vm_ptr_container_.insert(vm_ptr->name(), vm_ptr);
		return true;
	}
	return false;
}

bool ScopeVisitor::insert_fm(const FunctionManager* fm_ptr) {
	if (is_effective_scope(fm_ptr) == true) {
		fm_ptr_container_.insert(fm_ptr->name(), fm_ptr);
		return true;
	}
	return false;
}

std::optional<ScopeVisitor::IdentifierType> ScopeVisitor::identify(
	const std::string& name
) const {
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
	return std::nullopt;
}

std::optional<ScopeVisitor::ConstDataPtr> ScopeVisitor::find_variable(
	const std::string& name
) const{
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		auto is_var = vm_ptr_container_[i]->find(name);
		if (is_var != std::nullopt) {
			return is_var.value();
		}
	}
	return std::nullopt;
}

std::optional<ScopeVisitor::StructProxy> ScopeVisitor::find_type(
	const std::string& name
)const {
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		auto is_type = vm_ptr_container_[i]->find_type(name);
		if (is_type != std::nullopt) {
			return is_type.value();
		}
	}
	return std::nullopt;
}

std::optional<ScopeVisitor::FunctionPtr> ScopeVisitor::find_function(
	const std::string& name
) const {
	std::size_t size = fm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		auto is_func = fm_ptr_container_[i]->find(name);
		if (is_func != std::nullopt) {
			return is_func.value();
		}
	}
	return std::nullopt;
}

bool ScopeVisitor::copy(const ScopeVisitor& that) {
	if (this == &that) {
		return true;
	}
	auto v_visitors = that.vm_ptr_container_.get_visitor();
	std::size_t size = v_visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		if (is_effective_scope(*v_visitors[i].second) == true) {
			vm_ptr_container_.emplace(*v_visitors[i].first, *v_visitors[i].second);
		}
		else {
			return false;
		}
	}
	auto f_visitors = that.fm_ptr_container_.get_visitor();
	size = f_visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		if (is_effective_scope(*f_visitors[i].second) == true) {
			fm_ptr_container_.emplace(*f_visitors[i].first, *f_visitors[i].second);
		}
		else {
			return false;
		}
	}
	return true;
}

bool ScopeVisitor::is_effective_scope(
	const VariableManager* vm_ptr
) {
	if (vm_ptr == nullptr) {
#ifdef  SCRIPT_DEBUG
		SCRIPT_CERR
			<< "ScopeVisitor : VariableManager指针为空" << std::endl;
		ASSERT(false);
#endif // SCRIPT_DEBUG
		return false;
	}
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (vm_ptr_container_[i]->has_name_conflict(*vm_ptr) == true) {
#ifdef SCRIPT_DEBUG
			SCRIPT_CERR
				<< "ScopeVisitor名称冲突 : VariableManager名称冲突"
				<< vm_ptr_container_[i]->name() << "和" << vm_ptr->name()
				<< "中存在名称冲突" << std::endl;
			ASSERT(false);
#endif // SCRIPT_DEBUG	
			return false;
		}
	}

	size = fm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (fm_ptr_container_[i]->has_name_conflict(*vm_ptr) == true) {
#ifdef SCRIPT_DEBUG
			SCRIPT_CERR
				<< "ScopeVisitor名称冲突 : FunctionManager"
				<< fm_ptr_container_[i]->name() << "和VariableManager" << vm_ptr->name()
				<< "中存在名称冲突" << std::endl;
			ASSERT(false);
#endif // SCRIPT_DEBUG
			return false;
		}
	}
	return true;
}

bool ScopeVisitor::is_effective_scope(
	const FunctionManager* fm_ptr
) {
	if (fm_ptr == nullptr) {
#ifdef  SCRIPT_DEBUG
		SCRIPT_CERR
			<< "ScopeVisitor名称冲突 : FunctionManager指针为空" << std::endl;
		ASSERT(false);
#endif // SCRIPT_DEBUG
		return false;
	}
	std::size_t size = vm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (vm_ptr_container_[i]->has_name_conflict(*fm_ptr) == true) {
#ifdef SCRIPT_DEBUG
			SCRIPT_CERR
				<< "ScopeVisitor名称冲突 : VariableManager"
				<< vm_ptr_container_[i]->name() << "和FunctionManager" << fm_ptr->name()
				<< "中存在名称冲突" << std::endl;
			ASSERT(false);
#endif // SCRIPT_DEBUG
			return false;
		}
	}

	size = fm_ptr_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (fm_ptr_container_[i]->has_name_conflict(*fm_ptr) == true) {
#ifdef SCRIPT_DEBUG
			SCRIPT_CERR
				<< "ScopeVisitor名称冲突 : FunctionManager名称冲突"
				<< fm_ptr_container_[i]->name() << "和" << fm_ptr->name()
				<< "中存在名称冲突" << std::endl;
			ASSERT(false);
#endif // SCRIPT_DEBUG
			return false;
		}
	}
	return true;
}
