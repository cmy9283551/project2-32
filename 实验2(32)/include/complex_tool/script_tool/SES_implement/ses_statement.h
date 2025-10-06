#pragma once

#include "ses_syntax_tree.h"

namespace ses {
	class StatementNode : public AbstractSyntaxTree {
	public:
		virtual ~StatementNode() = default;
	};

	//�ű���,��Ӧ{...},������
	class StmtBlockNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtBlock;
	};

	//���ʽ,��Ӧ�ֺŽ�β�����
	class StmtExpressionNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtExpression;
	};

	//����,��Ӧ�����������
	class StmtDeclarationNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtDeclaration;
	};

	//if���
	class StmtIfNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtIf;
	};

	//while���
	class StmtWhileNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtWhile;
	};

	//for���
	class StmtForNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtFor;
	};

	//break���
	class StmtBreakNode : public StatementNode {
	public:
	private:
		static const ASTType  type_ = ASTType::StmtBreak;
	};

	//continue���
	class StmtContinueNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtContinue;
	};

	//return���
	class StmtReturnNode : public StatementNode {
	public:
	private:
		static const ASTType type_ = ASTType::StmtReturn;
	};
}