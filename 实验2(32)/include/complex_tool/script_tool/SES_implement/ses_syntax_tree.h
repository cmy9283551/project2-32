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

	//�﷨����,�����﷨��

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

	//�ű����е�һ�������ű�
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