#pragma once

#include <memory>
#include <string>

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
		StmtReturn

		//Expression
	};

	struct SourceLocation {
		std::size_t line;
	};

	class ASTVisitor;

	//语法解析,构建语法树

	class AbstractSyntaxTree {
	public:
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
		void visit(ASTVisitor& visitor) override;
		ASTType type() const override;
	private:
		static const ASTType type_ = ASTType::Script;

		std::unique_ptr<AbstractSyntaxTree> root_;
		std::unique_ptr<ScriptConfig> config_;
		std::string script_name_;
	};
}