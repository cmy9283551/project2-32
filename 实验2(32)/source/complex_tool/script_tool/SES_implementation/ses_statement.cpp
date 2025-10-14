#include "complex_tool/script_tool/SES_implementation/ses_statement.h"

#include "complex_tool/script_tool/SES_implementation/ses_ast_visitor.h"

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
		visitor.visit(*this);
	}

	ASTNodeType StmtBlockNode::type() const {
		return type_;
	}

	const std::vector<std::unique_ptr<AbstractSyntaxTree>>& StmtBlockNode::ast_nodes() const{
		return ast_nodes_;
	}

	StmtExpressionNode::StmtExpressionNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> expression
	) :StatementNode(location), expression_(std::move(expression)) {
	}

	void StmtExpressionNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtExpressionNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& StmtExpressionNode::expression() const {
		return *expression_;
	}

	StmtDeclarationNode::StmtDeclarationNode(
		const SourceLocation& location,
		const std::string& type_name,
		const std::string& var_name,
		std::unique_ptr<AbstractSyntaxTree> init_value,
		bool is_const
	) : StatementNode(location),
		type_name_(type_name),
		var_name_(var_name),
		init_value_(std::move(init_value)), is_const_(is_const) {
	}

	void StmtDeclarationNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtDeclarationNode::type() const {
		return type_;
	}

	const std::string& StmtDeclarationNode::type_name() {
		return type_name_;
	}

	const std::string& StmtDeclarationNode::var_name() {
		return var_name_;
	}

	const std::unique_ptr<AbstractSyntaxTree>& StmtDeclarationNode::init_value()const {
		return init_value_;
	}

	bool StmtDeclarationNode::is_const() const {
		return is_const_;
	}

	StmtAssignmentNode::StmtAssignmentNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> target,
		Token::TokenType op,
		std::unique_ptr<AbstractSyntaxTree> value
	) : StatementNode(location),
		target_(std::move(target)), op_(op), value_(std::move(value)) {
	}

	void StmtAssignmentNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtAssignmentNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& StmtAssignmentNode::target() const {
		return *target_;
	}

	Token::TokenType StmtAssignmentNode::op() const {
		return op_;
	}

	AbstractSyntaxTree& StmtAssignmentNode::value() const {
		return *value_;
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

	ASTNodeType StmtIfNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& StmtIfNode::condition() const {
		return *condition_;
	}

	AbstractSyntaxTree& StmtIfNode::then_branch() const {
		return *then_branch_;
	}

	const std::unique_ptr<AbstractSyntaxTree>& StmtIfNode::else_branch() const {
		return else_branch_;
	}

	StmtWhileNode::StmtWhileNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> condition,
		std::unique_ptr<AbstractSyntaxTree> body
	) : StatementNode(location),
		condition_(std::move(condition)), body_(std::move(body)) {

	}

	void StmtWhileNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtWhileNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& StmtWhileNode::condition() const {
		return *condition_;
	}

	AbstractSyntaxTree& StmtWhileNode::body() const {
		return *body_;
	}

	StmtBreakNode::StmtBreakNode(const SourceLocation& location)
		:StatementNode(location) {
	}

	void StmtBreakNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtBreakNode::type() const {
		return type_;
	}

	StmtContinueNode::StmtContinueNode(const SourceLocation& location)
		:StatementNode(location) {
	}

	void StmtContinueNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtContinueNode::type() const {
		return type_;
	}

	StmtReturnNode::StmtReturnNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> value
	) :StatementNode(location),
		value_(std::move(value)) {
	}

	void StmtReturnNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType StmtReturnNode::type() const {
		return type_;
	}

	const std::unique_ptr<AbstractSyntaxTree>& StmtReturnNode::value() const {
		return value_;
	}

}