#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

//��Žű�������Ϣ
struct SESScriptConfig {
	//��������Ĳ���
	SESScriptParameter input, output;

	SESModuleVisitor module_visitor;
	ScopeVisitor scope_visitor;
};

//���ģ��������Ϣ
struct SESModuleConfig {
	ScopeVisitor scope_visitor;
};

//��ʾ��������еĻ�����Ϣ
//����һ��ģ��Ľű�����ʱ,��ģ����������ģ��,������Χ
//ͨ��Ҳ�Ǹ�ģ���ܷ��ʵ����Χ
struct SESCompileDependence {
	ScopeVisitor scope_visitor;
	SESModuleVisitor module_visitor;
	SESScriptConfig default_script_config;
	SESModuleConfig default_module_config;
};

//����AST
//���߳���,��ÿ���������໥����,���ö��������ͬʱ����
//���Զ��ʹ�ý�������ļ�
class SESParser {
public:
	virtual ~SESParser() = default;
	SESParser();
	//���ڽ���*.ses�ļ�,���ؽű��е����ж����ű�
	virtual std::vector<std::unique_ptr<SESAbstractSyntaxTree>> parse_ses(
		const std::string& script_path,
		const SESCompileDependence& dependence
	);
protected:
	using TokenType = SESToken::TokenType;

	//ѡ��ֻ�ģʽ����ֹ����
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
	bool is_at_end()const;
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

	//�ֻ�ģʽ��������
	void panic_mode(PanicEnd end);
private:
	//���ڲ��ַ���,ֱ�Ӷ�������ĸ÷��ż���
	void panic_mode_common(PanicEnd end);
	//����),],},Ҫ���Ƕ��Ƕ��
	void panic_mode_mult(PanicEnd end);

	SESParser* parent_parser_ = nullptr;
};

//���ڽ����ű����õ��ӽ�����
class SESParser::ConfigParser {
public:
	using PanicEnd = SESParser::PanicEnd;
	using ParameterType = SESScriptParameter::ParameterType;

	ConfigParser(SESParser* parent_parser);

	std::shared_ptr<SESScriptConfig> parse_ses_script_config();
private:
	SESToken current_token()const;
	void advance();
	bool check(TokenType type)const;
	bool match(TokenType type);
	bool is_at_end()const;
	void panic_mode_recovery(PanicEnd end);
	const std::string& current_file_path()const;
	const std::string& current_script_name()const;

	bool parse_module_list(std::vector<std::string>& module_list);
	bool parse_variable_scope(std::vector<std::string>& variable_scope);
	bool parse_function_scope(std::vector<std::string>& function_scope);
	bool parse_parameter(SESScriptParameter& parameter);

	void analysis_scope(
		std::vector<std::string>& module_list,
		std::vector<std::string>& variable_scope,
		std::vector<std::string>& function_scope,
		std::shared_ptr<SESScriptConfig> config
	)const;

	enum class Keyword {
		Module,
		Input,
		OutPut,
		VariableScope,
		FunctionScope,
		Option
	};
	static const std::unordered_map<std::string, Keyword> keyword_list_;

	SESParser* parent_parser_ = nullptr;
};

//ʹ�õݹ��½������﷨������
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

//ʹ��Pratt���������﷨������
class SESPrattParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
};
