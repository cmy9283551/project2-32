#pragma once

#include "ses_ast_visitor.h"

namespace ses {
	//局部变量表,用于语义分析
	//记录变量的类型,是否为常量等信息
	//但不进行变量内存分配等内存管理工作
	//避免与生成器功能耦合
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

	//语义分析器,此处实现对Script和Module通用功能的解析
	//但对两种具体脚本的语义分析工作,交由各自的子类实现
	//由于Script和Module的配置结构不同,所以需要分开处理
	//而且分开后,可以更好地控制各自的功能
	//语义分析器的职责:
	//1.检查变量是否声明,类型是否匹配,作用域是否正确
	//2.检查函数调用是否正确
	//3.检查控制流语句的正确性
	//4.检查表达式的类型是否匹配
	//5.为每个表达式节点设置其类型信息
	//6.为每个变量节点设置其变量信息
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

	//解析一个文件中的所有单元
	//未来可能会加入多线程分析单元的功能(因为单元之间相互独立)
	class BatchSemanticAnalyzer {
	public:
		BatchSemanticAnalyzer(const SemanticAnalyzerDependency& dependency);
		~BatchSemanticAnalyzer() = default;

		void analyze_script(const std::vector<std::unique_ptr<AbstractSyntaxTree>>& units);
	private:
		SemanticAnalyzerDependency dependency_;
	};
}