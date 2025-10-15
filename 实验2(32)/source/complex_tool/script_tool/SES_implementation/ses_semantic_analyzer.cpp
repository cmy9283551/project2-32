#include "complex_tool/script_tool/SES_implementation/ses_semantic_analyzer.h"

namespace ses {

	SemanticAnalyzer::SemanticAnalyzer(
		const SemanticAnalyzerDependency& dependency
	) :dependency_(dependency) {
	}

	void SemanticAnalyzer::visit(StmtBlockNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtExpressionNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtDeclarationNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtAssignmentNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtIfNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtWhileNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtForNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtBreakNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtContinueNode& node)
	{
	}
	void SemanticAnalyzer::visit(StmtReturnNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprUnaryNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprFuncNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprCallNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprLiteralNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprVariableNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprMemberNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprIndexNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprBinaryNode& node)
	{
	}
	void SemanticAnalyzer::visit(ExprInitializerNode& node)
	{
	}
	ScriptSemanticAnalyzer::ScriptSemanticAnalyzer(
		const SemanticAnalyzerDependency& dependency
	):SemanticAnalyzer(dependency)
	{
	}

	void ScriptSemanticAnalyzer::visit(ScriptNode& node)
	{
	}
	
	BatchSemanticAnalyzer::BatchSemanticAnalyzer(
		const SemanticAnalyzerDependency& dependency
	) :dependency_(dependency)
	{
	}

	void BatchSemanticAnalyzer::analyze_script(
		const std::vector<std::unique_ptr<AbstractSyntaxTree>>& units
	)
	{
	}

}