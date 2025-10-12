#include "complex_tool/script_tool/SES_implementation/ses_statement.h"

namespace ses {
	StmtBlockNode::StmtBlockNode(
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& ast_node
	) :ast_nodes_(std::move(ast_node)) {
	}

	void StmtBlockNode::visit(ASTVisitor& visitor) {
		//TO DO
	}

	ASTType StmtBlockNode::type() const {
		return type_;
	}
}