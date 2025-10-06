#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

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

	//��ʾ��������еĻ�����Ϣ
	//����һ��ģ��Ľű�����ʱ,��ģ����������ģ��,������Χ
	//ͨ��Ҳ�Ǹ�ģ���ܷ��ʵ����Χ
	struct CompileDependence {
		ScopeVisitor scope_visitor;
		ModuleVisitor module_visitor;
		ScriptConfig default_script_config;
		ModuleConfig default_module_config;
	};

	//����AST
	//���߳���,��ÿ���������໥����,���ö��������ͬʱ����
	//���Զ��ʹ�ý�������ļ�
	class Parser {
	public:
		virtual ~Parser() = default;
		Parser();
		//���ڽ���*.ses�ļ�,���ؽű��е����ж����ű�
		virtual std::vector<std::unique_ptr<AbstractSyntaxTree>> parse_ses(
			const std::string& script_path,
			const CompileDependence& dependence
		);
	protected:
		using TokenType = Token::TokenType;

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

		std::unique_ptr<AbstractSyntaxTree> parse_ses_script();
		Token current_token()const;
		void advance();
		bool check(TokenType type)const;
		bool match(TokenType type);
		bool is_at_end()const;
		void panic_mode_recovery(PanicEnd end);

		virtual std::unique_ptr<SESStatementNode> parse_ses_statement()const = 0;

		std::string current_file_path_;
		std::string current_script_name_;
		TokenStream* current_token_stream_ = nullptr;
		const CompileDependence* current_dependence_ = nullptr;
		const ScriptConfig* current_script_config_ = nullptr;

		std::unique_ptr<ErrorRecoverer> error_recoerer_;
		std::unique_ptr<ConfigParser> config_parser_;
	};

	class Parser::ErrorRecoverer {
	public:
		using PanicEnd = Parser::PanicEnd;

		ErrorRecoverer(Parser* parent_parser);

		//�ֻ�ģʽ��������
		void panic_mode(PanicEnd end);
	private:
		//���ڲ��ַ���,ֱ�Ӷ�������ĸ÷��ż���
		void panic_mode_common(PanicEnd end);
		//����),],},Ҫ���Ƕ��Ƕ��
		void panic_mode_mult(PanicEnd end);

		Parser* parent_parser_ = nullptr;
	};

	//���ڽ����ű����õ��ӽ�����
	class Parser::ConfigParser {
	public:
		using PanicEnd = Parser::PanicEnd;
		using ParameterType = ScriptParameter::ParameterType;

		ConfigParser(Parser* parent_parser);

		std::shared_ptr<ScriptConfig> parse_ses_script_config();
	private:
		Token current_token()const;
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
		bool parse_parameter(ScriptParameter& parameter);

		void analysis(
			std::vector<std::string>& module_list,
			std::vector<std::string>& variable_scope,
			std::vector<std::string>& function_scope,
			std::shared_ptr<ScriptConfig> config
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

		Parser* parent_parser_ = nullptr;
	};

	//ʹ�õݹ��½������﷨������
	class RecursiveDescentParser : public Parser {
	protected:
		class SESExpressionParser;
		class SESStatementParser;

		std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
	};

	class RecursiveDescentParser::SESExpressionParser {
	public:
	private:
		Parser* parent_parser_ = nullptr;
	};

	class RecursiveDescentParser::SESStatementParser {
	public:
	private:
		Parser* parent_parser_ = nullptr;
	};

	//ʹ��Pratt���������﷨������
	class PrattParser : public Parser {
	protected:
		std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
	};
}