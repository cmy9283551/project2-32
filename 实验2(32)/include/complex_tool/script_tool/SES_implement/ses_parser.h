#pragma once

#include "ses_lexer.h"
#include "ses_expression.h"

namespace ses {

#define SCRIPT_SES_PARSER_LOG

#define SCRIPT_PARSER_THROW_ERROR(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__);

#define SCRIPT_PARSER_THROW_ERROR_HIDE_TOKEN(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__,false);

#define SCRIPT_PARSER_CONSUME(token,message)\
	consume(token,message,__LINE__,__func__);

	struct ParserErrorMessage {
		Token error_token;
		std::string message;
		std::size_t line;
		std::string func;
		bool show_token = true;
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
		Parser(const CompileDependence& dependence);
		//���ڽ���*.ses�ļ�,���ؽű��е����ж����ű�
		virtual std::optional<std::vector<std::unique_ptr<AbstractSyntaxTree>>> parse_ses(
			const std::string& script_path
		);
	protected:
		using TokenType = Token::TokenType;
		using StructTemplateContainer = VariableManager::StructTemplateContainer;

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

		enum class Precedence {
			Assign,  // =, +=, -=
			LogicalOr,  // ||
			LogicalAns, // &&
			Equality,    // ==, !=
			Relational,  // <, >, <=, >=
			Additive,    // +, -
			Multiplicative, // *, /, %
			Unary,       // +, -, ++, --
			Postfix     // [], (), .
		};

		//token��ǩ,����ɸѡtoken
		enum class TokenTag {
			Keyword,
			TypeName,
			Declaration,
			Constant,
			Null
		};
		TokenTag find_tag(TokenType type)const;
		bool check_tag(TokenTag tag)const;

		enum class IdentifierType {
			LocalVar,
			ModuleType,
			ModuleFunc,
			InternalType,
			InternalFunc,
			InternalVar,
			Null
		};
		//������ǰidentifier������
		IdentifierType identify();

		StructTemplateContainer& current_stc();

		Token current_token()const;
		void advance();
		bool check(TokenType type)const;
		bool check(const std::vector<TokenType>& type);
		bool match(TokenType type);
		void consume(
			TokenType type, 
			const std::string& message, 
			std::size_t line, const 
			std::string& func
		);
		bool is_at_end()const;
		void panic_mode_recovery(PanicEnd end);

		std::optional<std::unique_ptr<AbstractSyntaxTree>> parse_ses_script();
		virtual std::unique_ptr<AbstractSyntaxTree> parse_ses_statement() = 0;

		std::string current_file_path_;
		std::string current_script_name_;
		std::vector<LocalVariableTable> variable_stack_;
		//�˴�ָ����,��ָ֤��ѵ�ָ����ʹ������ָ��
		std::unique_ptr<TokenStream> current_token_stream_ = nullptr;
		std::unique_ptr<ScriptConfig> current_script_config_ = nullptr;

		const CompileDependence* dependence_ = nullptr;
		std::unique_ptr<ErrorRecoverer> error_recoerer_ = nullptr;
		std::unique_ptr<ConfigParser> config_parser_ = nullptr;
	};

	class Parser::ErrorRecoverer {
	public:
		using PanicEnd = Parser::PanicEnd;

		ErrorRecoverer(Parser& parent_parser);

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

		ConfigParser(Parser& parent_parser);

		std::unique_ptr<ScriptConfig> parse_ses_script_config();
	private:
		Token current_token()const;
		void advance();
		bool check(TokenType type)const;
		bool match(TokenType type);
		void consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);
		bool is_at_end()const;
		const std::string& current_file_path()const;
		const std::string& current_script_name()const;

		void parse_module_list(std::vector<std::string>& module_list);
		void parse_variable_scope(std::vector<std::string>& variable_scope);
		void parse_function_scope(std::vector<std::string>& function_scope);
		void parse_parameter(ScriptParameter& parameter);

		void analysis(
			std::vector<std::string>& module_list,
			std::vector<std::string>& variable_scope,
			std::vector<std::string>& function_scope,
			std::unique_ptr<ScriptConfig>& config
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

		//��֤�ӽ�������������С�ڸ�������
		Parser* parent_parser_ = nullptr;
	};

	//ʹ�õݹ��½������﷨������
	class RecursiveDescentParser : public Parser {
	public:
		RecursiveDescentParser(const CompileDependence& dependence);
	protected:
		class ExpressionParser;

		std::unique_ptr<AbstractSyntaxTree> parse_ses_statement() override;

		std::unique_ptr<AbstractSyntaxTree> parse_block();
		std::unique_ptr<AbstractSyntaxTree> parse_variable_declaration();

		std::unique_ptr<ExpressionParser> expression_parser_ = nullptr;
	};

	class RecursiveDescentParser::ExpressionParser {
	public:
		ExpressionParser(RecursiveDescentParser& parent_parser);
	private:
		Token current_token()const;
		void advance();
		bool check(TokenType type)const;
		bool check(const std::vector<TokenType>& type)const;
		bool match(TokenType type);
		void consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);
		bool is_at_end()const;
		const std::string& current_file_path()const;
		const std::string& current_script_name()const;

		RecursiveDescentParser* parent_parser_ = nullptr;
	};

	//ʹ��Pratt���������﷨������
	class PrattParser : public Parser {
	protected:
		std::unique_ptr<AbstractSyntaxTree> parse_ses_statement() override;
	};
}