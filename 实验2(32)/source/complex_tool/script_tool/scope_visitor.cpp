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
	const std::vector<std::pair<std::string, const VariableManager*>>& vm_ptr_list,
	const std::vector<std::pair<std::string, const FunctionManager*>>& fm_ptr_list
) :vm_ptr_container_(vm_ptr_list), fm_ptr_container_(fm_ptr_list) {
}

std::optional<ScopeVisitor::ScopeNotFound> ScopeVisitor::init_sub_scope(
	const std::vector<std::string>& vm_list,
	const std::vector<std::string>& fm_list,
	ScopeVisitor& sub_scope
) const {
	bool success = true;
	ScopeNotFound message;
	std::size_t size = vm_list.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = vm_ptr_container_.find(vm_list[i]);
		if (iter == vm_ptr_container_.cend()) {
			success = false;
			message.variable_scope.push_back(vm_list[i]);
			continue;
		}
		sub_scope.vm_ptr_container_.emplace(iter.first(), iter.second());
	}

	size = fm_list.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = fm_ptr_container_.find(fm_list[i]);
		if (iter == fm_ptr_container_.cend()) {
			success = false;
			message.function_scope.push_back(fm_list[i]);
			continue;
		}
		sub_scope.fm_ptr_container_.emplace(iter.first(), iter.second());
	}

	if (success == true) {
		return std::nullopt;
	}
	return message;
}

std::optional<ScopeVisitor::ScopeNotFound> ScopeVisitor::init_sub_scope(
	const ScopeList& scope_list, ScopeVisitor& sub_scope
) const {
	return init_sub_scope(scope_list.variable_scope, scope_list.function_scope, sub_scope);
}

void ScopeVisitor::get_scope_list(
	std::vector<std::string>& vm_list, std::vector<std::string>& fm_list
) const {
	//保证有序,因为顺序会决定覆盖的先后
	auto viter = vm_ptr_container_.cbegin();
	std::vector<decltype(viter)> iters;
	for (; viter != vm_ptr_container_.cend(); ++viter) {
		iters.emplace_back(viter);
	}
	std::sort(iters.begin(), iters.end(), [](const auto& x, const auto& y) {
		return x.position() < y.position();
		});
	for (std::size_t i = 0; i < iters.size(); i++) {
		vm_list.push_back(iters[i].first());
	}

	auto fiter = fm_ptr_container_.cbegin();
	std::vector<decltype(fiter)> fiters;
	for (; fiter != fm_ptr_container_.cend(); ++fiter) {
		fiters.emplace_back(fiter);
	}
	std::sort(fiters.begin(), fiters.end(), [](const auto& x, const auto& y) {
		return x.position() < y.position();
		});
	for (std::size_t i = 0; i < fiters.size(); i++) {
		fm_list.push_back(fiters[i].first());
	}
}

void ScopeVisitor::get_scope_list(ScopeList& scope_list) const {
	get_scope_list(scope_list.variable_scope, scope_list.function_scope);
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