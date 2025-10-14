#pragma once

#include "ses_statement.h"

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
		ASTNodeType type()const override;
	private:
		Token::TokenType op_;
		std::unique_ptr<AbstractSyntaxTree> operand_ = nullptr;

		static const ASTNodeType type_ = ASTNodeType::ExprUnary;
	};

	class ExprFuncNode : public ExpressionNode {
	public:
		ExprFuncNode(
			const SourceLocation& location,
			const std::string& callee
		);
		~ExprFuncNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::string callee_;

		static const ASTNodeType type_ = ASTNodeType::ExprFunc;
	};

	class ExprCallNode : public ExpressionNode {
	public:
		ExprCallNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> callee,
			std::vector<std::unique_ptr<AbstractSyntaxTree>>& params
		);
		~ExprCallNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> callee_ = nullptr;
		std::vector<std::unique_ptr<AbstractSyntaxTree>> params_;

		static const ASTNodeType type_ = ASTNodeType::ExprCall;
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
		ASTNodeType type()const override;
	private:
		bool parse_value(
			const std::string& value,
			LiteralType type
		);

		std::variant<
			ScriptInt, ScriptFloat, ScriptChar, ScriptString, bool
		> value_;
		LiteralType literal_type_;

		static const ASTNodeType type_ = ASTNodeType::ExprLiteral;
	};

	class ExprVariableNode : public ExpressionNode {
	public:
		ExprVariableNode(
			const SourceLocation& location,
			const std::string& var_name
		);
		~ExprVariableNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::string var_name_;

		static const ASTNodeType type_ = ASTNodeType::ExprVariable;
	};

	class ExprMemberNode : public ExpressionNode {
	public:
		ExprMemberNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> object,
			const std::string& member_name
		);
		~ExprMemberNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> object_ = nullptr;
		std::string member_name_;

		static const ASTNodeType type_ = ASTNodeType::ExprMember;
	};

	class ExprIndexNode : public ExpressionNode {
	public:
		ExprIndexNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> array,
			std::unique_ptr<AbstractSyntaxTree> index
		);
		~ExprIndexNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> array_ = nullptr;
		std::unique_ptr<AbstractSyntaxTree> index_ = nullptr;

		static const ASTNodeType type_ = ASTNodeType::ExprIndex;
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
		ASTNodeType type()const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> left_ = nullptr;
		Token::TokenType op_;
		std::unique_ptr<AbstractSyntaxTree> right_ = nullptr;

		static const ASTNodeType type_ = ASTNodeType::ExprBinary;
	};

	class ExprInitializerNode : public ExpressionNode {
	public:
		ExprInitializerNode(
			const SourceLocation& location,
			std::vector<std::unique_ptr<AbstractSyntaxTree>>& values
		);
		~ExprInitializerNode() = default;

		void visit(ASTVisitor& visitor)override;
		ASTNodeType type()const override;
	private:
		std::vector<std::unique_ptr<AbstractSyntaxTree>> values_;
		static const ASTNodeType type_ = ASTNodeType::ExprInitializer;
	};
}