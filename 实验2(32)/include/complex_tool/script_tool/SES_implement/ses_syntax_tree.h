#pragma once

#include <memory>
#include <string>

#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_module.h"

namespace ses {
	//存放脚本配置信息
	struct ScriptConfig {
		//输入输出的参数
		ScriptParameter input, output;

		ModuleVisitor module_visitor;
		ScopeVisitor scope_visitor;
	};

	//存放模组配置信息
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