#include "complex_tool/script_tool/SES_implementation/ses_syntax_tree.h"

#include "complex_tool/script_tool/SES_implementation/ses_ast_visitor.h"

namespace ses {
	AbstractSyntaxTree::AbstractSyntaxTree(const SourceLocation& location)
		:location_(location) {
	}
	const SourceLocation& AbstractSyntaxTree::location() const {
		return location_;
	}

	const std::string& AbstractSyntaxTree::node_type_to_string(
		ASTNodeType type
	){
		static const std::unordered_map<ASTNodeType, std::string> type_to_string_map = {
			{ASTNodeType::Script,"Script"},
			//Statement
			{ASTNodeType::StmtBlock,"StmtBlock"},
			{ASTNodeType::StmtExpression,"StmtExpression"},
			{ASTNodeType::StmtDeclaration,"StmtDeclaration"},
			{ASTNodeType::StmtAssignment,"StmtAssignment"},
			{ASTNodeType::StmtIf,"StmtIf"},
			{ASTNodeType::StmtWhile,"StmtWhile"},
			{ASTNodeType::StmtFor,"StmtFor"},
			{ASTNodeType::StmtBreak,"StmtBreak"},
			{ASTNodeType::StmtContinue,"StmtContinue"},
			{ASTNodeType::StmtReturn,"StmtReturn"},
			//Expression
			{ASTNodeType::ExprUnary,"ExprUnary"},
			{ASTNodeType::ExprFunc,"ExprFunc"},
			{ASTNodeType::ExprCall,"ExprCall"},
			{ASTNodeType::ExprLiteral,"ExprLiteral"},
			{ASTNodeType::ExprVariable,"ExprVariable"},
			{ASTNodeType::ExprMember,"ExprMember"},
			{ASTNodeType::ExprIndex,"ExprIndex"},
			{ASTNodeType::ExprBinary,"ExprBinary"},
			{ASTNodeType::ExprInitializer,"ExprInitializer"},
		};
		auto iter = type_to_string_map.find(type);
		if (iter != type_to_string_map.end()) {
			return iter->second;
		}
		static const std::string unknown_type = "UnknownType";
		return unknown_type;
	}

	ScriptNode::ScriptNode(
		const SourceLocation& location,
		const std::string& name,
		std::unique_ptr<AbstractSyntaxTree> root,
		std::unique_ptr<ScriptConfig> config
	) :AbstractSyntaxTree(location),
		script_name_(name), root_(std::move(root)), config_(std::move(config)) {
	}

	void ScriptNode::visit(ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	ASTNodeType ScriptNode::type() const {
		return type_;
	}

	const std::string& ScriptNode::script_name() const{
		return script_name_;
	}

	AbstractSyntaxTree& ScriptNode::root() const {
		return *root_;
	}

	const ScriptConfig& ScriptNode::config() const {
		return *config_;
	}

}