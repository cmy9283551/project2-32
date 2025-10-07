#include "complex_tool/script_tool/SES_implement/ses_syntax_tree.h"

namespace ses {
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

	const SourceLocation& AbstractSyntaxTree::location() const {
		return location_;
	}

	ScriptNode::ScriptNode(
		const std::string& name,
		std::unique_ptr<AbstractSyntaxTree> root,
		std::unique_ptr<ScriptConfig> config
	) :script_name_(name), root_(std::move(root)), config_(std::move(config)) {
	}

	void ScriptNode::visit(ASTVisitor& visitor) {
		//TO DO
	}

	ASTType ScriptNode::type() const {
		return type_;
	}
}