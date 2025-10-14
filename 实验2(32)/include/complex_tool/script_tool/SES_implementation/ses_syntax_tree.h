#pragma once

#include <memory>
#include <string>

#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_script.h"

namespace ses {
	enum class ASTNodeType {
		Script,

		//Statement
		StmtBlock,
		StmtExpression,
		StmtDeclaration,
		StmtAssignment,
		StmtIf,
		StmtWhile,
		StmtFor,
		StmtBreak,
		StmtContinue,
		StmtReturn,

		//Expression
		ExprUnary,
		ExprFunc,
		ExprCall,
		ExprLiteral,
		ExprVariable,
		ExprMember,
		ExprIndex,
		ExprBinary,
		ExprInitializer,
	};

	struct SourceLocation {
		std::string unit_name;
		std::size_t line = 0;
	};

	class ASTVisitor;

	//�﷨����,�����﷨��

	class AbstractSyntaxTree {
	public:
		AbstractSyntaxTree(const SourceLocation& location);
		virtual ~AbstractSyntaxTree() = default;

		virtual void visit(ASTVisitor& visitor) = 0;
		virtual ASTNodeType type()const = 0;

		const SourceLocation& location()const;
	protected:
		SourceLocation location_;
	};

	struct ScriptConfig;
	struct ModuleConfig;

	//�ű����е�һ�������ű�
	class ScriptNode : public AbstractSyntaxTree {
	public:
		ScriptNode(
			const SourceLocation& location,
			const std::string& name,
			std::unique_ptr<AbstractSyntaxTree> root,
			std::unique_ptr<ScriptConfig> config
		);
		void visit(ASTVisitor& visitor) override;
		ASTNodeType type() const override;
	private:
		static const ASTNodeType type_ = ASTNodeType::Script;

		std::string script_name_;
		std::unique_ptr<AbstractSyntaxTree> root_;
		std::unique_ptr<ScriptConfig> config_;
	};
}