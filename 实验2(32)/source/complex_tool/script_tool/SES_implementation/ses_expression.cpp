#include "complex_tool/script_tool/SES_implementation/ses_expression.h"

namespace ses {

	ExpressionNode::ExpressionNode(const SourceLocation& location)
		: AbstractSyntaxTree(location) {
	}

	ExprUnaryNode::ExprUnaryNode(
		const SourceLocation& location,
		Token::TokenType op,
		std::unique_ptr<AbstractSyntaxTree> operand
	)	: ExpressionNode(location), op_(op), operand_(std::move(operand)) {
	}

	void ExprUnaryNode::visit(ASTVisitor& visitor) {
	}

	ASTType ExprUnaryNode::type() const {
		return type_;
	}

	ExprLiteralNode::ExprLiteralNode(
		const SourceLocation& location, 
		LiteralType type, 
		const std::string& value,
		bool& success
	): ExpressionNode(location), literal_type_(type) {
		success = parse_value(value, type);
	}

	void ExprLiteralNode::visit(ASTVisitor& visitor)
	{
	}

	ASTType ExprLiteralNode::type() const {
		return type_;
	}

	bool ExprLiteralNode::parse_value(const std::string& value, LiteralType type){
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

	ExprLocalVarNode::ExprLocalVarNode(
		const SourceLocation& location, 
		const StructProxy& var_type, 
		const std::string& var_name
	): ExpressionNode(location), var_type_(var_type), var_name_(var_name) {
	}

	void ExprLocalVarNode::visit(ASTVisitor& visitor){
	}

	ASTType ExprLocalVarNode::type() const{
		return type_;
	}
	ExprInternalVarNode::ExprInternalVarNode(
		const SourceLocation& location, 
		const ConstDataPtr& data_ptr,
		const std::string& var_name
	): ExpressionNode(location), data_ptr_(data_ptr),var_name_(var_name) {
	}

	void ExprInternalVarNode::visit(ASTVisitor& visitor){
	}

	ASTType ExprInternalVarNode::type() const{
		return type_;
	}
	ExprBinaryNode::ExprBinaryNode(
		const SourceLocation& location, 
		std::unique_ptr<AbstractSyntaxTree> left, 
		Token::TokenType op, 
		std::unique_ptr<AbstractSyntaxTree> right
	): ExpressionNode(location), 
		left_(std::move(left)), op_(op), right_(std::move(right)) {
	}

	void ExprBinaryNode::visit(ASTVisitor& visitor){
	}

	ASTType ExprBinaryNode::type() const{
		return type_;
	}
}