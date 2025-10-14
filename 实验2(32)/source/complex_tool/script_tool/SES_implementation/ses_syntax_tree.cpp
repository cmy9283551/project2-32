#include "complex_tool/script_tool/SES_implementation/ses_syntax_tree.h"

namespace ses {
	AbstractSyntaxTree::AbstractSyntaxTree(const SourceLocation& location)
		:location_(location) {
	}
	const SourceLocation& AbstractSyntaxTree::location() const {
		return location_;
	}

	ScriptNode::ScriptNode(
		const SourceLocation& location,
		const std::string& name,
		std::unique_ptr<AbstractSyntaxTree> root,
		std::unique_ptr<ScriptConfig> config
	) :AbstractSyntaxTree(location),
		script_name_(name), root_(std::move(root)), config_(std::move(config)) {
	}

	void ScriptNode::visit(ASTVisitor& visitor) {
		//TO DO
	}

	ASTNodeType ScriptNode::type() const {
		return type_;
	}

}