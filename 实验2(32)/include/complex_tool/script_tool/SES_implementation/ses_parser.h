#pragma once

#include "ses_lexer.h"
#include "ses_expression.h"

#include <list>

namespace ses {

#define SCRIPT_SES_PARSER_LOG

#define SCRIPT_PARSER_THROW_ERROR(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__);

#define SCRIPT_PARSER_THROW_ERROR_HIDE_TOKEN(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__,false);

	//���ڴ洢һ����������õ�����ʱ����
	class LocalVariableTable {
	public:
		using StructTemplateContainer = VariableManager::StructTemplateContainer;
		LocalVariableTable() = default;
		LocalVariableTable(StructTemplateContainer& struct_template_container);

		bool push_back(const std::string& type_name, const std::string& var_name);
	private:
		StructTemplateContainer* struct_template_container_;
		//������->�������ʹ���
		IndexedMap<std::string, std::size_t> variable_table_;
	};

	struct ParserErrorMessage {
		Token error_token;
		std::string message;
		std::size_t line;
		std::string func;
		bool show_token = true;
	};

	//��ʾ���������еĻ�����Ϣ
	//����һ��ģ��Ľű�����ʱ,��ģ����������ģ��,������Χ
	//ͨ��Ҳ�Ǹ�ģ���ܷ��ʵ����Χ
	struct ParserDependence {
		const ScopeVisitor* scope_visitor;
		const ModuleVisitor* module_visitor;
		const ScriptConfig* default_script_config;
		const ModuleConfig* default_module_config;
	};

	//����AST
	//���߳���,��ÿ���������໥����,���ö��������ͬʱ����
	//���Զ��ʹ�ý�������ļ�
	//�̰߳�ȫ��:InstanceSafe
	class Parser {
	public:
		virtual ~Parser() = default;
		Parser(const ParserDependence& dependence);
		std::optional<std::vector<std::unique_ptr<AbstractSyntaxTree>>> parse(
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

		class ChildParser;
		class ErrorRecoverer;
		class StatementParser;

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
			ControlFlow,
			Const,
			TypeName,
			Constant,

			LocalVar,
			ModuleType,
			ModuleFunc,
			InternalType,
			InternalFunc,
			InternalVar,
			Identifier,

			NoTag
		};

		virtual StructTemplateContainer& current_stc() = 0;
		virtual const std::string& current_unit_name()const = 0;
		virtual std::optional<TokenTag> identify()const = 0;
		virtual std::optional<std::unique_ptr<AbstractSyntaxTree>> parse_unit() = 0;

		//������ǰidentifier������
		TokenTag find_tag(TokenType type)const;
		bool check_tag(TokenTag tag)const;
		bool check_tag(const std::vector<TokenTag>& tag)const;
		bool match_tag(TokenTag tag);

		Token current_token()const;
		void advance();
		bool check(TokenType type)const;
		bool check(const std::vector<TokenType>& type);
		bool match(TokenType type);
		Token consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);
		bool is_at_end()const;
		void panic_mode_recovery(PanicEnd end);

		//�ֲ��������ջ,���ڴ��������������
		//��listģ��ջ,�������,��������Է��ʷ�ջ��Ԫ��
		std::list<LocalVariableTable> variable_stack_;
		std::string current_file_path_;
		std::unique_ptr<TokenStream> current_token_stream_ = nullptr;

		std::unique_ptr<ErrorRecoverer> error_recoerer_ = nullptr;
		std::unique_ptr<StatementParser> statement_parser_ = nullptr;
		const ParserDependence* dependence_ = nullptr;
	};

	class ScriptParser :public Parser {
	public:
		ScriptParser(const ParserDependence& dependence);
		~ScriptParser() = default;
	private:
		class ScriptConfigParser;

		StructTemplateContainer& current_stc() override;
		const std::string& current_unit_name()const override;
		std::optional<TokenTag> identify()const override;

		std::optional<std::unique_ptr<AbstractSyntaxTree>> parse_unit() override;

		std::string current_script_name_;
		//�˴�ָ����,��ָ֤��ѵ�ָ����ʹ������ָ��
		std::unique_ptr<ScriptConfig> current_script_config_ = nullptr;

		std::unique_ptr<ScriptConfigParser> script_config_parser_ = nullptr;
	};

	class Parser::ChildParser {
	public:
		ChildParser(Parser& parent_parser);
		virtual ~ChildParser() = default;
	protected:
		Token current_token()const;
		void advance();



		TokenTag find_tag(TokenType type)const;
		bool check_tag(TokenTag tag)const;
		bool check_tag(const std::vector<TokenTag>& tag)const;
		bool match_tag(TokenTag tag);

		bool check(TokenType type)const;
		bool match(TokenType type);
		Token consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);

		bool is_at_end()const;

		//��ǰ�ű������ͱ�
		StructTemplateContainer& current_stc();
		std::list<LocalVariableTable>& current_variable_stack();
		const std::string& current_file_path()const;
		const std::string& current_unit_name()const;

		const ParserDependence* dependence()const;

		Parser* parent_parser_ = nullptr;
	};

	class Parser::ErrorRecoverer :public Parser::ChildParser {
	public:
		ErrorRecoverer(Parser& parent_parser);
		~ErrorRecoverer() = default;
		//�ֻ�ģʽ��������
		void panic_mode(PanicEnd end);
	private:
		//���ڲ��ַ���,ֱ�Ӷ�������ĸ÷��ż���
		void panic_mode_common(PanicEnd end);
		//����),],},Ҫ���Ƕ��Ƕ��
		void panic_mode_mult(PanicEnd end);
	};

	//���ڽ����ű����õ��ӽ�����
	class ScriptParser::ScriptConfigParser :public ScriptParser::ChildParser {
	public:
		using ParameterType = ScriptParameter::ParameterType;

		ScriptConfigParser(Parser& parent_parser);
		~ScriptConfigParser() = default;

		std::unique_ptr<ScriptConfig> parse_ses_script_config();
	private:
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
	class Parser::StatementParser : public Parser::ChildParser {
	public:
		StatementParser(Parser& parent_parser);

		std::unique_ptr<AbstractSyntaxTree> parse_statement();
	private:
		std::unique_ptr<AbstractSyntaxTree> parse_block();
		std::unique_ptr<AbstractSyntaxTree> parse_variable_declaration();
		std::unique_ptr<AbstractSyntaxTree> parse_control_flow();
		std::unique_ptr<AbstractSyntaxTree> parse_expression();
	};
}