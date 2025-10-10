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

		//在创建时根据使用的类型自动构建,
		VariableManager::StructTemplateContainer script_stc;

		friend std::ostream& operator<<(std::ostream& os, const ScriptConfig& config);
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

	//用于存储一个代码块中用到的临时变量
	class LocalVariableTable {
	public:
		using StructTemplateContainer = VariableManager::StructTemplateContainer;
		LocalVariableTable() = default;
		LocalVariableTable(StructTemplateContainer& struct_template_container);

		bool push_back(const std::string& type_name,const std::string& var_name);
	private:
		StructTemplateContainer* struct_template_container_;
		IndexedMap<std::string, std::size_t> variable_table_;
	};

	struct SourceLocation {
		std::size_t line = 0;
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