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
		out_ << "-NodeInfo: script's name:" << node.script_name() << "\n";
		node.root().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtBlockNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Block with " << node.ast_nodes().size() << " statements:\n";
		for (const auto& stmt : node.ast_nodes()) {
			stmt->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtExpressionNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Expression Statement\n";
		node.expression().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtDeclarationNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Declaration of variable '" << node.var_name()
			<< "' of type '" << node.type_name() << "'"
			<< (node.is_const() ? " (const)" : "") << "\n";
		if (node.init_value() != nullptr) {
			print_indent();
			out_ << "-NodeInfo: Init Value:\n";
			node.init_value()->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtAssignmentNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Assignment with operator '"
			<< Token::token_type_to_string(node.op()) << "'\n";
		print_indent();
		out_ << "-NodeInfo: Target:\n";
		node.target().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Value:\n";
		node.value().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtIfNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: If Statement\n";
		print_indent();
		out_ << "-NodeInfo: Condition:\n";
		node.condition().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Then Branch:\n";
		node.then_branch().visit(*this);
		if (node.else_branch() != nullptr) {
			print_indent();
			out_ << "-NodeInfo: Else Branch:\n";
			node.else_branch()->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtWhileNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: While Statement\n";
		print_indent();
		out_ << "-NodeInfo: Condition:\n";
		node.condition().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Body:\n";
		node.body().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtForNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: For Statement\n";
		// For loop details would go here if available
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtBreakNode& node) {
		print_node_basic_info(node);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtContinueNode& node) {
		print_node_basic_info(node);
		indent_level_--;
	}

	void DebugASTVisitor::visit(StmtReturnNode& node) {
		print_node_basic_info(node);
		print_indent();
		if (node.value() != nullptr) {
			print_indent();
			out_ << "-NodeInfo: Return Value:\n";
			node.value()->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprUnaryNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Unary Operator '" << Token::token_type_to_string(node.op()) << "'\n";
		print_indent();
		out_ << "-NodeInfo: Operand:\n";
		node.operand().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprFuncNode& node) {
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Function Reference to '" << node.callee() << "'\n";
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprCallNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Callee:\n";
		node.callee().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Parameters (" << node.params().size() << "):\n";
		for (const auto& param : node.params()) {
			param->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprLiteralNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Literal of type '";
		switch (node.literal_type()) {
		case ExprLiteralNode::LiteralType::Int:
			out_ << "Int' with value " << std::get<ScriptInt>(node.value()) << "\n";
			break;
		case ExprLiteralNode::LiteralType::Float:
			out_ << "Float' with value " << std::get<ScriptFloat>(node.value()) << "\n";
			break;
		case ExprLiteralNode::LiteralType::Char:
			out_ << "Char' with value '" << std::get<ScriptChar>(node.value()) << "'\n";
			break;
		case ExprLiteralNode::LiteralType::String:
			out_ << "String' with value \"" << std::get<ScriptString>(node.value()) << "\"\n";
			break;
		case ExprLiteralNode::LiteralType::Bool:
			out_ << "Bool' with value " << (std::get<bool>(node.value()) ? "true" : "false") << "\n";
			break;
		default:
			out_ << "Unknown'\n";
			break;
		}
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprVariableNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Variable Reference to '" << node.var_name() << "'\n";
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprMemberNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Object:\n";
		node.object().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Member Access of '" << node.member_name() << "'\n";
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprIndexNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Array:\n";
		node.array().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Index:\n";
		node.index().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprBinaryNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Binary Operator '" << Token::token_type_to_string(node.op()) << "'\n";
		print_indent();
		out_ << "-NodeInfo: Left Operand:\n";
		node.left().visit(*this);
		print_indent();
		out_ << "-NodeInfo: Right Operand:\n";
		node.right().visit(*this);
		indent_level_--;
	}

	void DebugASTVisitor::visit(ExprInitializerNode& node){
		print_node_basic_info(node);
		print_indent();
		out_ << "-NodeInfo: Initializer with " << node.values().size() << " values\n";
		for (const auto& value : node.values()) {
			value->visit(*this);
		}
		indent_level_--;
	}

	void DebugASTVisitor::print_node_basic_info(const AbstractSyntaxTree& node) {
		print_indent();
		out_ << "-NodeType: " << node.node_type_to_string(node.type()) << "\n";
		//自动增加缩进
		indent_level_++;
		print_node_location(node);
	}

	void DebugASTVisitor::print_node_location(const AbstractSyntaxTree& node) {
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
