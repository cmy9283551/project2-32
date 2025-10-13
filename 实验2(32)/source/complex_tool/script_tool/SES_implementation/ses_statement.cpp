#include "complex_tool/script_tool/SES_implementation/ses_statement.h"

namespace ses {

	StatementNode::StatementNode(const SourceLocation& location)
		:AbstractSyntaxTree(location) {
	}

	StmtBlockNode::StmtBlockNode(
		const SourceLocation& location,
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& ast_node
	) : StatementNode(location),
		ast_nodes_(std::move(ast_node)) {
	}

	void StmtBlockNode::visit(ASTVisitor& visitor) {
		//TO DO
	}

	ASTType StmtBlockNode::type() const {
		return type_;
	}

	StmtExpressionNode::StmtExpressionNode(
		const SourceLocation& location, 
		std::unique_ptr<AbstractSyntaxTree> expression
	):StatementNode(location), expression_(std::move(expression)) {
	}

	void StmtExpressionNode::visit(ASTVisitor& visitor) {
	}

	ASTType StmtExpressionNode::type() const {
		return type_;
	}

	StmtDeclarationNode::StmtDeclarationNode(
		const SourceLocation& location,
		const VariableManager::StructProxy& var_type,
		const std::string& var_name,
		std::unique_ptr<AbstractSyntaxTree> init_value,
		bool is_const
	) : StatementNode(location),
		var_type_(var_type), var_name_(var_name),
		init_value_(std::move(init_value)), is_const_(is_const) {
	}

	void StmtDeclarationNode::visit(ASTVisitor& visitor) {

	}

	ASTType StmtDeclarationNode::type() const {
		return type_;
	}
	StmtIfNode::StmtIfNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> condition,
		std::unique_ptr<AbstractSyntaxTree> then_branch,
		std::unique_ptr<AbstractSyntaxTree> else_branch
	) :StatementNode(location),
		condition_(std::move(condition)),
		then_branch_(std::move(then_branch)),
		else_branch_(std::move(else_branch)) {
	}

	void StmtIfNode::visit(ASTVisitor& visitor) {
	}

	ASTType StmtIfNode::type() const {
		return type_;
	}

	StmtWhileNode::StmtWhileNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> condition,
		std::unique_ptr<AbstractSyntaxTree> body
	) : StatementNode(location),
		condition_(std::move(condition)), body_(std::move(body)) {

	}

	void StmtWhileNode::visit(ASTVisitor& visitor) {

	}

	ASTType StmtWhileNode::type() const {
		return type_;
	}

	StmtBreakNode::StmtBreakNode(const SourceLocation& location)
		:StatementNode(location) {
	}

	void StmtBreakNode::visit(ASTVisitor& visitor)
	{
	}

	ASTType StmtBreakNode::type() const {
		return type_;
	}

	StmtContinueNode::StmtContinueNode(const SourceLocation& location)
		:StatementNode(location) {
	}

	void StmtContinueNode::visit(ASTVisitor& visitor)
	{
	}

	ASTType StmtContinueNode::type() const {
		return type_;
	}

	StmtReturnNode::StmtReturnNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> value
	) :StatementNode(location),
		value_(std::move(value)) {
	}

	void StmtReturnNode::visit(ASTVisitor& visitor) {

	}

	ASTType StmtReturnNode::type() const {
		return type_;
	}

}