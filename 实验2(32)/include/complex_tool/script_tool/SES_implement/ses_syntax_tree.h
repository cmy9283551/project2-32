#pragma once

#include <memory>
#include <string>

#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_module.h"

namespace ses {
	//��Žű�������Ϣ
	struct ScriptConfig {
		//��������Ĳ���
		ScriptParameter input, output;

		ModuleVisitor module_visitor;
		ScopeVisitor scope_visitor;
	};

	//���ģ��������Ϣ
	struct ModuleConfig {
		ScopeVisitor scope_visitor;
	};

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
		ScriptNode(
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