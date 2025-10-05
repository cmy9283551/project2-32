#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

//存放脚本配置信息
struct SESScriptConfig {
	//输入输出的参数
	SESScriptParameter input, output;

	SESModuleVisitor module_visitor;
	ScopeVisitor scope_visitor;
};

//存放模组配置信息
struct SESModuleConfig {
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

//生成AST
//单线程类,但每个解析器相互独立,可让多个解析器同时解析
//可以多次使用解析多个文件
class SESParser {
public:
	virtual ~SESParser() = default;
	SESParser();
	//用于解析*.ses文件,返回脚本中的所有独立脚本
	virtual std::vector<std::unique_ptr<SESAbstractSyntaxTree>> parse_ses(
		const std::string& script_path,
		const SESCompileDependence& dependence
	);
protected:
	using TokenType = SESToken::TokenType;

	//选择恐慌模式的终止符号
	enum class PanicEnd {
		RightParen,//-> )
		RightBracket,//-> ]
		RightBrace,//-> }
		Semicolon,//-> ;
		Comma//-> ,
	};

	class ErrorRecoverer;
	class ConfigParser;

	std::unique_ptr<SESAbstractSyntaxTree> parse_ses_script();
	SESToken current_token()const;
	void advance();
	bool check(TokenType type)const;
	bool match(TokenType type);

	void panic_mode_recovery(PanicEnd end);

	virtual std::unique_ptr<SESStatementNode> parse_ses_statement()const = 0;

	std::string current_file_path_;
	std::string current_script_name_;
	SESTokenStream* current_token_stream_ = nullptr;
	const SESCompileDependence* current_dependence_ = nullptr;
	const SESScriptConfig* current_script_config_ = nullptr;

	std::unique_ptr<ErrorRecoverer> error_recoerer_;
	std::unique_ptr<ConfigParser> config_parser_;
};

class SESParser::ErrorRecoverer {
public:
	using PanicEnd = SESParser::PanicEnd;

	ErrorRecoverer(SESParser* parent_parser);

	//恐慌模式跳过错误
	void panic_mode(PanicEnd end);
private:
	//对于部分符号,直接读到最近的该符号即可
	void panic_mode_common(PanicEnd end);
	//对于),],},要考虑多层嵌套
	void panic_mode_mult(PanicEnd end);

	SESParser* parent_parser_ = nullptr;
};

//用于解析脚本配置的子解析器
class SESParser::ConfigParser {
public:
	using PanicEnd = SESParser::PanicEnd;

	ConfigParser(SESParser* parent_parser);

	std::unique_ptr<SESScriptConfig> parse_ses_script_config()const;
private:
	void parse_module_list(SESModuleVisitor& module_list);
	void parse_variable_scope(std::vector<std::string>& variable_scope);
	void parse_function_scope(std::vector<std::string>& function_scope);
	void parse_input_parameter(SESScriptParameter& input);
	void parse_output_parameter(SESScriptParameter& output);

	SESParser* parent_parser_ = nullptr;
};

//使用递归下降法的语法解析器
class SESRecursiveDescentParser : public SESParser {
protected:
	class SESExpressionParser;
	class SESStatementParser;

	std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
};

class SESRecursiveDescentParser::SESExpressionParser {
public:
private:
	SESParser* parent_parser_ = nullptr;
};

class SESRecursiveDescentParser::SESStatementParser {
public:
private:
	SESParser* parent_parser_ = nullptr;
};

//使用Pratt解析法的语法解析器
class SESPrattParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
};
