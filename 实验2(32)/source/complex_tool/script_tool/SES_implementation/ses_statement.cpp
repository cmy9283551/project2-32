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

	StmtDeclarationNode::StmtDeclarationNode(
		const VariableManager::StructProxy& var_type,
		const std::string& var_name,
		std::unique_ptr<AbstractSyntaxTree> init_value,
		bool is_const
	) : var_type_(var_type), var_name_(var_name),
		init_value_(std::move(init_value)), is_const_(is_const) {
	}

	void StmtDeclarationNode::visit(ASTVisitor& visitor) {

	}

	ASTType StmtDeclarationNode::type() const {
		return type_;
	}
	StmtIfNode::StmtIfNode(
		std::unique_ptr<AbstractSyntaxTree> condition,
		std::unique_ptr<AbstractSyntaxTree> then_branch,
		std::unique_ptr<AbstractSyntaxTree> else_branch
	) : condition_(std::move(condition)),
		then_branch_(std::move(then_branch)),
		else_branch_(std::move(else_branch)) {
	}

	void StmtIfNode::visit(ASTVisitor& visitor) {
	}

	ASTType StmtIfNode::type() const {
		return type_;
	}
}