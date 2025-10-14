#include "complex_tool/script_tool/SES_implementation/ses_ast_visitor.h"

namespace ses {
	const std::string& ASTVisitor::node_type_to_string(ASTNodeType type) {
		return AbstractSyntaxTree::node_type_to_string(type);
	}

	DebugASTVisitor::DebugASTVisitor(std::ostream& out)
		:out_(out) {
	}

	void DebugASTVisitor::visit(ScriptNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: ScriptName" << node.script_name() << "\n";
		node.root().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtBlockNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Block with " << node.ast_nodes().size() << " statements\n";
		for (const auto& stmt : node.ast_nodes()) {
			stmt->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtExpressionNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Expression Statement\n";
		node.expression().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtDeclarationNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Declaration of variable '" << node.var_name()
			<< "' of type '" << node.type_name() << "'"
			<< (node.is_const() ? " (const)" : "") << "\n";
		node.init_value(*this);
		indent_level_--;
	}

	void DebugASTVisitor::print_node_basic_info(const AbstractSyntaxTree& node){
		print_indent();
		out_ << "-NodeType: " << node.node_type_to_string(node.type()) << "\n";
		//自动增加缩进
		indent_level_++;
		print_node_location(node);
	}

	void DebugASTVisitor::print_node_location(const AbstractSyntaxTree& node){
		print_indent();
		out_ << "-Location: " 
			<< node.location().unit_name << " (line " << node.location().line << ")\n";
	}

	void DebugASTVisitor::print_indent() {
		for (std::size_t i = 0; i < indent_level_; i++) {
			out_ << "	";
		}
	}
}
