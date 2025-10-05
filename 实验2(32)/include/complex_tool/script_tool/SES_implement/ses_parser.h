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
