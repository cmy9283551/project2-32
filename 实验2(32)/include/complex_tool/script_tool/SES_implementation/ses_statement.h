#pragma once

#include "ses_syntax_tree.h"

namespace ses {

	class StatementNode : public AbstractSyntaxTree {
	public:
		StatementNode(const SourceLocation& location);
		virtual ~StatementNode() = default;
	};

	//脚本块,对应{...},存放语句
	class StmtBlockNode : public StatementNode {
	public:
		StmtBlockNode(
			const SourceLocation& location,
			std::vector<std::unique_ptr<AbstractSyntaxTree>>& ast_node
		);
		~StmtBlockNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		static const ASTNodeType type_ = ASTNodeType::StmtBlock;

		std::vector<std::unique_ptr<AbstractSyntaxTree>> ast_nodes_;
	};

	//表达式,对应分号结尾的语句
	class StmtExpressionNode : public StatementNode {
	public:
		StmtExpressionNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> expression
		);
		~StmtExpressionNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> expression_ = nullptr;

		static const ASTNodeType type_ = ASTNodeType::StmtExpression;
	};

	//声明,对应变量声明语句
	class StmtDeclarationNode : public StatementNode {
	public:
		StmtDeclarationNode(
			const SourceLocation& location,
			const std::string& type_name,
			const std::string& var_name,
			std::unique_ptr<AbstractSyntaxTree> init_value = nullptr,
			bool is_const = false
		);
		~StmtDeclarationNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		using StructProxy = VariableManager::StructProxy;

		std::string type_name_;
		std::string var_name_;
		std::unique_ptr<AbstractSyntaxTree> init_value_ = nullptr;
		bool is_const_ = false;

		static const ASTNodeType type_ = ASTNodeType::StmtDeclaration;
	};

	//if语句
	class StmtIfNode : public StatementNode {
	public:
		StmtIfNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> condition,
			std::unique_ptr<AbstractSyntaxTree> then_branch,
			std::unique_ptr<AbstractSyntaxTree> else_branch = nullptr
		);
		~StmtIfNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> condition_ = nullptr;
		std::unique_ptr<AbstractSyntaxTree> then_branch_ = nullptr;
		std::unique_ptr<AbstractSyntaxTree> else_branch_ = nullptr;
		static const ASTNodeType type_ = ASTNodeType::StmtIf;
	};

	//while语句
	class StmtWhileNode : public StatementNode {
	public:
		StmtWhileNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> condition,
			std::unique_ptr<AbstractSyntaxTree> body
		);

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> condition_ = nullptr;
		std::unique_ptr<AbstractSyntaxTree> body_ = nullptr;
		static const ASTNodeType type_ = ASTNodeType::StmtWhile;
	};

	//for语句
	class StmtForNode : public StatementNode {
	public:
	private:
		static const ASTNodeType type_ = ASTNodeType::StmtFor;
	};

	//break语句
	class StmtBreakNode : public StatementNode {
	public:
		StmtBreakNode(const SourceLocation& location);
		~StmtBreakNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		static const ASTNodeType  type_ = ASTNodeType::StmtBreak;
	};

	//continue语句
	class StmtContinueNode : public StatementNode {
	public:
		StmtContinueNode(const SourceLocation& location);
		~StmtContinueNode() = default;

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		static const ASTNodeType type_ = ASTNodeType::StmtContinue;
	};

	//return语句
	class StmtReturnNode : public StatementNode {
	public:
		StmtReturnNode(
			const SourceLocation& location,
			std::unique_ptr<AbstractSyntaxTree> value
		);

		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		std::unique_ptr<AbstractSyntaxTree> value_ = nullptr;

		static const ASTNodeType type_ = ASTNodeType::StmtReturn;
	};
}