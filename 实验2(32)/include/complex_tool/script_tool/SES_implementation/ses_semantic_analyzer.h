#pragma once

#include "ses_ast_visitor.h"

namespace ses {
	//�ֲ�������,�����������
	//��¼����������,�Ƿ�Ϊ��������Ϣ
	//�������б����ڴ������ڴ������
	//�������������������
	class LocalVariableTable {
	public:
	private:
		StructTemplateContainer* struct_template_container_ = nullptr;
		IndexedMap<std::string, std::size_t> var_table_;
	};

	struct SemanticAnalyzerDependency {
		const ModuleVisitor* module_visitor = nullptr;
		const ScopeVisitor* scope_visitor = nullptr;
	};

	//���������,�˴�ʵ�ֶ�Script��Moduleͨ�ù��ܵĽ���
	//�������־���ű��������������,���ɸ��Ե�����ʵ��
	//����Script��Module�����ýṹ��ͬ,������Ҫ�ֿ�����
	//���ҷֿ���,���Ը��õؿ��Ƹ��ԵĹ���
	//�����������ְ��:
	//1.�������Ƿ�����,�����Ƿ�ƥ��,�������Ƿ���ȷ
	//2.��麯�������Ƿ���ȷ
	//3.��������������ȷ��
	//4.�����ʽ�������Ƿ�ƥ��
	//5.Ϊÿ�����ʽ�ڵ�������������Ϣ
	//6.Ϊÿ�������ڵ������������Ϣ
	class SemanticAnalyzer :public ASTVisitor {
	public:
		SemanticAnalyzer(const SemanticAnalyzerDependency& dependency);
		virtual ~SemanticAnalyzer() = default;

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
	protected:
		SemanticAnalyzerDependency dependency_;
	};

	class ScriptSemanticAnalyzer : public SemanticAnalyzer {
	public:
		ScriptSemanticAnalyzer(const SemanticAnalyzerDependency& dependency);
		~ScriptSemanticAnalyzer() = default;

		void visit(ScriptNode& node) override;
	private:
	};

	//����һ���ļ��е����е�Ԫ
	//δ�����ܻ������̷߳�����Ԫ�Ĺ���(��Ϊ��Ԫ֮���໥����)
	class BatchSemanticAnalyzer {
	public:
		BatchSemanticAnalyzer(const SemanticAnalyzerDependency& dependency);
		~BatchSemanticAnalyzer() = default;

		void analyze_script(const std::vector<std::unique_ptr<AbstractSyntaxTree>>& units);
	private:
		SemanticAnalyzerDependency dependency_;
	};
}