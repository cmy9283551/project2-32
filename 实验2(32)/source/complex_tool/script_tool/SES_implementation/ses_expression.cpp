#include "complex_tool/script_tool/SES_implementation/ses_expression.h"

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
	}

	ASTNodeType ExprUnaryNode::type() const {
		return type_;
	}

	ExprFuncNode::ExprFuncNode(
		const SourceLocation& location,
		const std::string& callee
	) : ExpressionNode(location), callee_(callee) {
	}

	void ExprFuncNode::visit(ASTVisitor& visitor) {
	}

	ASTNodeType ExprFuncNode::type() const {
		return type_;
	}

	ExprCallNode::ExprCallNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> callee,
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& params
	) : ExpressionNode(location),
		callee_(std::move(callee)), params_(std::move(params)) {
	}

	void ExprCallNode::visit(ASTVisitor& visitor) {
	}

	ASTNodeType ExprCallNode::type() const {
		return type_;
	}

	ExprLiteralNode::ExprLiteralNode(
		const SourceLocation& location,
		LiteralType type,
		const std::string& value,
		bool& success
	) : ExpressionNode(location), literal_type_(type) {
		success = parse_value(value, type);
	}

	void ExprLiteralNode::visit(ASTVisitor& visitor)
	{
	}

	ASTNodeType ExprLiteralNode::type() const {
		return type_;
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
	}

	ASTNodeType ExprVariableNode::type() const {
		return type_;
	}

	ExprMemberNode::ExprMemberNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> object,
		const std::string& member_name
	) : ExpressionNode(location),
		object_(std::move(object)), member_name_(member_name) {
	}

	void ExprMemberNode::visit(ASTVisitor& visitor) {
	}

	ASTNodeType ExprMemberNode::type() const {
		return type_;
	}

	ExprIndexNode::ExprIndexNode(
		const SourceLocation& location,
		std::unique_ptr<AbstractSyntaxTree> array,
		std::unique_ptr<AbstractSyntaxTree> index
	) : ExpressionNode(location),
		array_(std::move(array)), index_(std::move(index)) {
	}

	void ExprIndexNode::visit(ASTVisitor& visitor) {
	}

	ASTNodeType ExprIndexNode::type() const {
		return type_;
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
	}

	ASTNodeType ExprBinaryNode::type() const {
		return type_;
	}

	ExprInitializerNode::ExprInitializerNode(
		const SourceLocation& location,
		std::vector<std::unique_ptr<AbstractSyntaxTree>>& values
	) : ExpressionNode(location), values_(std::move(values)) {
	}

	void ExprInitializerNode::visit(ASTVisitor& visitor) {
	}

	ASTNodeType ExprInitializerNode::type() const {
		return type_;
	}
}