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

	//表示编译过程中的环境信息
	//包含一个模块的脚本编译时,该模块给出的最大模组,作用域范围
	//通常也是该模块能访问的最大范围
	struct CompileDependence {
		ScopeVisitor scope_visitor;
		ModuleVisitor module_visitor;
		ScriptConfig default_script_config;
		ModuleConfig default_module_config;
	};

	//生成AST
	//单线程类,但每个解析器相互独立,可让多个解析器同时解析
	//可以多次使用解析多个文件
	class Parser {
	public:
		virtual ~Parser() = default;
		Parser(const CompileDependence& dependence);
		//用于解析*.ses文件,返回脚本中的所有独立脚本
		virtual std::optional<std::vector<std::unique_ptr<AbstractSyntaxTree>>> parse_ses(
			const std::string& script_path
		);
	protected:
		using TokenType = Token::TokenType;
		using StructTemplateContainer = VariableManager::StructTemplateContainer;

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

		//token标签,用于筛选token
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
		//解析当前identifier的类型
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
		//此处指针中,保证指向堆的指针已使用智能指针
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

		//恐慌模式跳过错误
		void panic_mode(PanicEnd end);
	private:
		//对于部分符号,直接读到最近的该符号即可
		void panic_mode_common(PanicEnd end);
		//对于),],},要考虑多层嵌套
		void panic_mode_mult(PanicEnd end);

		Parser* parent_parser_ = nullptr;
	};

	//用于解析脚本配置的子解析器
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

		//保证子解析器生命周期小于父解析器
		Parser* parent_parser_ = nullptr;
	};

	//使用递归下降法的语法解析器
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

	//使用Pratt解析法的语法解析器
	class PrattParser : public Parser {
	protected:
		std::unique_ptr<AbstractSyntaxTree> parse_ses_statement() override;
	};
}