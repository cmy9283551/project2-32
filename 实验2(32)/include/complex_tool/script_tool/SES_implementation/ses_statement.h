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
		StmtBlockNode(std::vector<std::unique_ptr<AbstractSyntaxTree>>& ast_node);

		void visit(ASTVisitor& visitor) override;
		ASTType type() const override;
	private:
		static const ASTType type_ = ASTType::StmtBlock;

		std::vector<std::unique_ptr<AbstractSyntaxTree>> ast_nodes_;
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
		bool is_const_ = false;
		std::string type_name_, var_name_;
		std::unique_ptr<AbstractSyntaxTree> init_value_ = nullptr;

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