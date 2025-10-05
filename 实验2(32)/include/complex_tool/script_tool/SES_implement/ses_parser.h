#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

//存放脚本配置信息
struct SESScriptConfig {
	//输入输出的参数
	SESScriptParameter input,output;

	SESModuleVisitor module_visitor;
	ScopeVisitor scope_visitor;
};

//存放模组配置信息
class SESModuleConfig {
	ScopeVisitor scope_visitor;
};

//表示编译过程中的环境信息
//包含一个模块的脚本编译时,该模块给出的最大模组,作用域范围
//通常也是该模块能访问的最大范围
struct SESCompileDependence {
	ScopeVisitor scope_visitor;
	SESModuleVisitor module_visitor;
	SESScriptConfig default_script_config;
	SESModuleConfig default_module_config;
};

class SESExpressionParser {
public:
private:

};

class SESStatementParser {
public:
private:
};

//生成AST
//单线程类,但每个解析器相互独立,可让多个解析器同时解析
//可以多次使用解析多个文件
class SESParser {
public:
	//用于解析*.ses文件,返回脚本中的所有独立脚本
	virtual std::vector<std::unique_ptr<SESAbstractSyntaxTree>> parse_ses(
		const std::string& script_path,
		const SESCompileDependence& dependence
	)const;
protected:
	std::unique_ptr<SESAbstractSyntaxTree> parse_ses_script(
		SESTokenStream& token_stream,
		const SESCompileDependence& dependence
	)const;

	virtual std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const = 0;
};

//使用递归下降法的语法解析器
class SESRecursiveDescentParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const override;
};

//使用Pratt解析法的语法解析器
class SESPrattParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const override;
};
