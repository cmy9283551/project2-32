#include "complex_tool/script_tool/SES_implementation/ses_script.h"

#include "complex_tool/script_tool/SES_implementation/ses_module.h"

namespace ses{
	std::ostream& operator<<(std::ostream& os, const ScriptConfig& config) {
		os << "脚本配置信息:\n"
			<< "传入参数:\n" << config.input
			<< "传出参数:\n" << config.output
			<< "载入模组:\n";
		std::vector<std::string> module_list;
		config.module_visitor.get_module_vector(module_list);
		std::size_t size = module_list.size();
		for (std::size_t i = 0; i < size; i++) {
			os << "[module]:" << module_list[i] << "\n";
		}
		os << "变量作用域:\n";
		ScopeVisitor::ScopeVector scope_list;
		config.scope_visitor.get_scope_vector(scope_list);
		size = scope_list.variable_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			os << "[variable manager]:" << scope_list.variable_scope[i] << "\n";
		}
		os << "函数作用域:\n";
		size = scope_list.function_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			os << "[function manager]:" << scope_list.function_scope[i] << "\n";
		}
		return os;
	}

}