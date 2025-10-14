#pragma once

#include "ses_statement.h"
#include "ses_expression.h"

namespace ses {
	//访问AST节点的访问器
	//Visitor模式
	class ASTVisitor {
	public:
		ASTVisitor() = default;
		virtual ~ASTVisitor() = default;

		//script
		virtual void visit(ScriptNode& node) = 0;

		//statement

		virtual void visit(StmtBlockNode& node) = 0;
		virtual void visit(StmtExpressionNode& node) = 0;
		virtual void visit(StmtDeclarationNode& node) = 0;
		virtual void visit(StmtAssignmentNode& node) = 0;
		virtual void visit(StmtIfNode& node) = 0;
		virtual void visit(StmtWhileNode& node) = 0;
		virtual void visit(StmtForNode& node) = 0;
		virtual void visit(StmtBreakNode& node) = 0;
		virtual void visit(StmtContinueNode& node) = 0;
		virtual void visit(StmtReturnNode& node) = 0;

		//expression

		virtual void visit(ExprUnaryNode& node) = 0;
		virtual void visit(ExprFuncNode& node) = 0;
		virtual void visit(ExprCallNode& node) = 0;
		virtual void visit(ExprLiteralNode& node) = 0;
		virtual void visit(ExprVariableNode& node) = 0;
		virtual void visit(ExprMemberNode& node) = 0;
		virtual void visit(ExprIndexNode& node) = 0;
		virtual void visit(ExprBinaryNode& node) = 0;
		virtual void visit(ExprInitializerNode& node) = 0;
	protected:
		static const std::string& node_type_to_string(ASTNodeType type);
	};

	class DebugASTVisitor : public ASTVisitor {
	public:
		DebugASTVisitor(std::ostream& out);
		~DebugASTVisitor() = default;

		void visit(ScriptNode& node) override;

		void visit(StmtBlockNode& node) override;
		void visit(StmtExpressionNode& node) override;
		void visit(StmtDeclarationNode& node) override;
		void visit(StmtAssignmentNode& node) override;
		void visit(StmtIfNode& node) override;
		void visit(StmtWhileNode& node) override;
		void visit(StmtForNode& node) override;
		void visit(StmtBreakNode& node) override;
		void visit(StmtContinueNode& node) override;
		void visit(StmtReturnNode& node) override;

		void visit(ExprUnaryNode& node) override;
		void visit(ExprFuncNode& node) override;
		void visit(ExprCallNode& node) override;
		void visit(ExprLiteralNode& node) override;
		void visit(ExprVariableNode& node) override;
		void visit(ExprMemberNode& node) override;
		void visit(ExprIndexNode& node) override;
		void visit(ExprBinaryNode& node) override;
		void visit(ExprInitializerNode& node) override;
	private:
		void print_node_basic_info(const AbstractSyntaxTree& node);
		void print_node_location(const AbstractSyntaxTree& node);
		void print_indent();

		std::size_t indent_level_ = 0;
		std::ostream& out_ = std::cout;
	};

}