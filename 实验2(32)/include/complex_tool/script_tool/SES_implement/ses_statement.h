#pragma once

#include "ses_syntax_tree.h"

namespace ses {
	class StatementNode : public AbstractSyntaxTree {
	public:
		virtual ~StatementNode() = default;
	};

	//脚本块,对应{...},存放语句
	class StmtBlockNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtBlock;
	};

	//表达式,对应分号结尾的语句
	class StmtExpressionNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtExpression;
	};

	//声明,对应变量声明语句
	class StmtDeclarationNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtDeclaration;
	};

	//if语句
	class StmtIfNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtIf;
	};

	//while语句
	class StmtWhileNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtWhile;
	};

	//for语句
	class StmtForNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtFor;
	};

	//break语句
	class StmtBreakNode : public StatementNode {
	public:
	private:
		static const ASTType  type_ = ASTType::StmtBreak;
	};

	//continue语句
	class StmtContinueNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtContinue;
	};

	//return语句
	class StmtReturnNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtReturn;
	};
}