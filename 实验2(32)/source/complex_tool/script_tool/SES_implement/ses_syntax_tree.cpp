#include "complex_tool/script_tool/SES_implement/ses_syntax_tree.h"

namespace ses {
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