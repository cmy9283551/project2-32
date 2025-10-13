#pragma once

#include "ses_statement.h"
#include "ses_lexer.h"

#include <variant>

namespace ses {
	//表达式节点
	class ExpressionNode : public AbstractSyntaxTree {
	public:
		ExpressionNode(const SourceLocation& location);
		virtual ~ExpressionNode() = default;
	};

	class ExprUnaryNode : public ExpressionNode {
	public:
		ExprUnaryNode(
			const SourceLocation& location,
			Token::TokenType op,
			std::unique_ptr<AbstractSyntaxTree> operand
		);
		~ExprUnaryNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTType type()const override;
	private:
		Token::TokenType op_;
		std::unique_ptr<AbstractSyntaxTree> operand_ = nullptr;

		static const ASTType type_ = ASTType::ExprUnary;
	};

	class ExprLiteralNode : public ExpressionNode {
	public:
		enum class LiteralType {
			Int,
			Float,
			Char,
			String,
			Bool
		};

		ExprLiteralNode(
			const SourceLocation& location, 
			LiteralType type,
			const std::string& value,
			bool& success
		);
		~ExprLiteralNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTType type()const override;
	private:
		bool parse_value(
			const std::string& value,
			LiteralType type
		);

		std::variant<
			ScriptInt, ScriptFloat, ScriptChar, ScriptString, bool
		> value_;
		LiteralType literal_type_;

		static const ASTType type_ = ASTType::ExprLiteral;
	};

	class ExprLocalVarNode : public ExpressionNode {
	public:
		using StructProxy = VariableManager::StructProxy;

		ExprLocalVarNode(
			const SourceLocation& location,
			const StructProxy& var_type,
			const std::string& var_name
		);
		~ExprLocalVarNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTType type()const override;
	private:
		StructProxy var_type_;
		std::string var_name_;

		static const ASTType type_ = ASTType::ExprLocalVar;
	};

	//注意,该变量虽存储方式与局部变量相同,但其实现方式与局部变量不同,必须区分
	class ExprInternalVarNode : public ExpressionNode {
	public:
		using StructProxy = VariableManager::StructProxy;
		using ConstDataPtr = VariableManager::ConstDataPtr;

		ExprInternalVarNode(
			const SourceLocation& location,
			const ConstDataPtr& data_ptr,
			const std::string& var_name
		);
		~ExprInternalVarNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTType type()const override;
	private:
		ConstDataPtr data_ptr_;
		std::string var_name_;

		static const ASTType type_ = ASTType::ExprInternalVar;
	};

	class ExprBinaryNode : public ExpressionNode {
	public:
		ExprBinaryNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> left,
			Token::TokenType op,
			std::unique_ptr<AbstractSyntaxTree> right
		);

		~ExprBinaryNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTType type()const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> left_ = nullptr;
		Token::TokenType op_;
		std::unique_ptr<AbstractSyntaxTree> right_ = nullptr;

		static const ASTType type_ = ASTType::ExprBinary;
	};

}