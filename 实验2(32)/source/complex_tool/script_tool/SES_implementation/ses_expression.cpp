#include "complex_tool/script_tool/SES_implementation/ses_expression.h"

#include "complex_tool/script_tool/SES_implementation/ses_ast_visitor.h"

namespace ses {

	ExpressionNode::ExpressionNode(const SourceLocation& location)
		: AbstractSyntaxTree(location) {
	}

	ExprUnaryNode::ExprUnaryNode(
		const SourceLocation& location,
		Token::TokenType op,
		std::unique_ptr<AbstractSyntaxTree> operand
	) : ExpressionNode(location), op_(op), operand_(std::move(operand)) {
	}

	void ExprUnaryNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprUnaryNode::type() const {
		return type_;
	}

	Token::TokenType ExprUnaryNode::op() const {
		return op_;
	}

	AbstractSyntaxTree& ExprUnaryNode::operand() const {
		return *operand_;
	}

	ExprFuncNode::ExprFuncNode(
		const SourceLocation& location,
		const std::string& callee
	) : ExpressionNode(location), callee_(callee) {
	}

	void ExprFuncNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprFuncNode::type() const {
		return type_;
	}

	const std::string& ExprFuncNode::callee() const {
		return callee_;
	}

	ExprCallNode::ExprCallNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> callee,
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& params
	) : ExpressionNode(location),
		callee_(std::move(callee)), params_(std::move(params)) {
	}

	void ExprCallNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprCallNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& ExprCallNode::callee() const {
		return *callee_;
	}

	const std::vector<std::unique_ptr<AbstractSyntaxTree>>& ExprCallNode::params() const {
		return params_;
	}

	ExprLiteralNode::ExprLiteralNode(
		const SourceLocation& location,
		LiteralType type,
		const std::string& value,
		bool& success
	) : ExpressionNode(location), literal_type_(type) {
		success = parse_value(value, type);
	}

	void ExprLiteralNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprLiteralNode::type() const {
		return type_;
	}

	const std::variant<
		ScriptInt, ScriptFloat, ScriptChar, ScriptString, bool
	>& ExprLiteralNode::value() const {
		return value_;
	}

	ExprLiteralNode::LiteralType ExprLiteralNode::literal_type() const {
		return literal_type_;
	}

	bool ExprLiteralNode::parse_value(const std::string& value, LiteralType type) {
		try {
			switch (type) {
			case LiteralType::Int:
				value_ = std::stoi(value);
				return true;
			case LiteralType::Float:
				value_ = std::stof(value);
				return true;
			case LiteralType::Char:
				if (value.size() != 1) {
					return false;
				}
				value_ = value[0];
				return true;
			case LiteralType::String:
				value_ = value;
				return true;
			case LiteralType::Bool:
				if (value == "true") {
					value_ = true;
					return true;
				}
				else if (value == "false") {
					value_ = false;
					return true;
				}
				return false;
			default:
				return false;
			}
		}
		catch (const std::exception&) {
			return false;
		}
	}

	ExprVariableNode::ExprVariableNode(
		const SourceLocation& location,
		const std::string& var_name
	) : ExpressionNode(location), var_name_(var_name) {
	}

	void ExprVariableNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprVariableNode::type() const {
		return type_;
	}

	const std::string& ExprVariableNode::var_name() const {
		return var_name_;
	}

	ExprMemberNode::ExprMemberNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> object,
		const std::string& member_name
	) : ExpressionNode(location),
		object_(std::move(object)), member_name_(member_name) {
	}

	void ExprMemberNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprMemberNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& ExprMemberNode::object() const {
		return *object_;
	}

	const std::string& ExprMemberNode::member_name() const {
		return member_name_;
	}

	ExprIndexNode::ExprIndexNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> array,
		std::unique_ptr<AbstractSyntaxTree> index
	) : ExpressionNode(location),
		array_(std::move(array)), index_(std::move(index)) {
	}

	void ExprIndexNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprIndexNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& ExprIndexNode::array() const {
		return *array_;
	}

	AbstractSyntaxTree& ExprIndexNode::index() const {
		return *index_;
	}

	ExprBinaryNode::ExprBinaryNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> left,
		Token::TokenType op,
		std::unique_ptr<AbstractSyntaxTree> right
	) : ExpressionNode(location),
		left_(std::move(left)), op_(op), right_(std::move(right)) {
	}

	void ExprBinaryNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprBinaryNode::type() const {
		return type_;
	}

	AbstractSyntaxTree& ExprBinaryNode::left() const {
		return *left_;
	}

	Token::TokenType ExprBinaryNode::op() const {
		return op_;
	}

	AbstractSyntaxTree& ExprBinaryNode::right() const {
		return *right_;
	}

	ExprInitializerNode::ExprInitializerNode(
		const SourceLocation& location,
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& values
	) : ExpressionNode(location), values_(std::move(values)) {
	}

	void ExprInitializerNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ExprInitializerNode::type() const {
		return type_;
	}
	const std::vector<std::unique_ptr<AbstractSyntaxTree>>& ExprInitializerNode::values(
	) const {
		return values_;
	}
}