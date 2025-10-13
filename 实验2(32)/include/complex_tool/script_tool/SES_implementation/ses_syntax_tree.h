#pragma once

#include <memory>
#include <string>

#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_script.h"

namespace ses {
	enum class ASTType {
		Script,

		//Statement
		StmtBlock,
		StmtExpression,
		StmtDeclaration,
		StmtIf,
		StmtWhile,
		StmtFor,
		StmtBreak,
		StmtContinue,
		StmtReturn,

		//Expression
		ExprUnary,
		ExprLiteral,
		ExprLocalVar,
		ExprInternalVar,
		ExprBinary,
	};

	struct SourceLocation {
		std::string unit_name;
		std::size_t line = 0;
	};

	class ASTVisitor;

	//语法解析,构建语法树

	class AbstractSyntaxTree {
	public:
		AbstractSyntaxTree(const SourceLocation& location);
		virtual ~AbstractSyntaxTree() = default;

		virtual void visit(ASTVisitor& visitor) = 0;
		virtual ASTType type()const = 0;

		const SourceLocation& location()const;
	protected:
		SourceLocation location_;
	};

	struct ScriptConfig;
	struct ModuleConfig;

	//脚本库中的一个独立脚本
	class ScriptNode : public AbstractSyntaxTree {
	public:
		ScriptNode(
			const SourceLocation& location,
			const std::string& name,
			std::unique_ptr<AbstractSyntaxTree> root,
			std::unique_ptr<ScriptConfig> config
		);
		void visit(ASTVisitor& visitor) override;
		ASTType type() const override;
	private:
		static const ASTType type_ = ASTType::Script;

		std::string script_name_;
		std::unique_ptr<AbstractSyntaxTree> root_;
		std::unique_ptr<ScriptConfig> config_;
	};
}