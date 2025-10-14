#pragma once

#include "ses_lexer.h"
#include "ses_expression.h"

#include <list>
#include <unordered_set>

namespace ses {

#define SCRIPT_PARSER_LOG

	struct ParserErrorMessage {
		Token error_token;
		std::string message;
		std::size_t line;
		std::string func;
		bool show_token = true;
	};

	//表示解析过程中的环境信息
	//包含一个模块的脚本编译时,该模块给出的最大模组,作用域范围
	//通常也是该模块能访问的最大范围
	struct ParserDependence {
		const ScopeVisitor* scope_visitor;
		const ModuleVisitor* module_visitor;
		const ScriptConfig* default_script_config;
		const ModuleConfig* default_module_config;
	};

	//生成AST
	//单线程类,但每个解析器相互独立,可让多个解析器同时解析
	//可以多次使用解析多个文件
	//线程安全性:InstanceSafe
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
		using StructProxy = VariableManager::StructProxy;

		class ChildParser;
		class ErrorRecoverer;
		class StatementParser;
		class ExpressionParser;

		//选择恐慌模式的终止符号
		enum class PanicEnd {
			RightParen,//-> )
			RightBracket,//-> ]
			RightBrace,//-> }
			Semicolon,//-> ;
			Comma//-> ,
		};

		//token标签,用于筛选token
		enum class TokenTag {
			ControlFlow,
			JumpFlow,
			Const,
			TypeName,

			Literal,
			Identifier,

			Primary,
			Postfix,
			Unary,
			Binary,
			Assignment,

			NoTag,
			MoreThanOneTag
		};

		//一个token可能有多个tag
		using TokenTagTable = std::unordered_set<TokenTag>;

		virtual StructTemplateContainer& current_stc() = 0;
		virtual const std::string& current_unit_name()const = 0;
		virtual const ScopeVisitor& current_scope_visitor()const = 0;
		virtual const ModuleVisitor& current_module_visitor()const = 0;
		virtual std::optional<std::unique_ptr<AbstractSyntaxTree>> parse_unit() = 0;

		bool is_type_name()const;
		//解析当前identifier的类型
		const TokenTagTable& find_tag(TokenType type)const;
		bool check_tag(TokenTag tag)const;
		bool check_tag(const std::vector<TokenTag>& tag)const;
		bool match_tag(TokenTag tag);

		const Token& current_token()const;
		const Token& look_ahead(std::size_t step = 1)const;
		void advance();
		bool check(TokenType type)const;
		bool check(const std::vector<TokenType>& type);
		bool match(TokenType type);
		const Token& consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);
		bool is_at_end()const;
		void panic_mode_recovery(PanicEnd end);

		std::string current_file_path_;
		std::unique_ptr<TokenStream> current_token_stream_ = nullptr;

		std::unique_ptr<ErrorRecoverer> error_recoerer_ = nullptr;
		std::unique_ptr<StatementParser> statement_parser_ = nullptr;
		std::unique_ptr<ExpressionParser> expression_parser_ = nullptr;
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
		const ScopeVisitor& current_scope_visitor()const override;
		const ModuleVisitor& current_module_visitor()const override;

		std::optional<std::unique_ptr<AbstractSyntaxTree>> parse_unit() override;

		std::string current_script_name_;
		//此处指针中,保证指向堆的指针已使用智能指针
		std::unique_ptr<ScriptConfig> current_script_config_ = nullptr;

		std::unique_ptr<ScriptConfigParser> script_config_parser_ = nullptr;
	};

	class Parser::ChildParser {
	public:
		ChildParser(Parser& parent_parser);
		virtual ~ChildParser() = default;
	protected:
		const Token& current_token()const;
		const Token& look_ahead(std::size_t step = 1)const;
		void advance();

		//识别当前标识符的标签
		//特殊情况:找不到标签返回std::nullopt,找到多个标签返回MoreThanOneTag
		//注意:此处的MoreThanOneTag是相对于传入的scope而言的
		//并非表示该token有多个tag
		TokenTag find_tag(TokenType type, const std::vector<TokenTag>& scope)const;
		bool check_tag(TokenTag tag)const;
		bool check_tag(const std::vector<TokenTag>& tag)const;
		bool match_tag(TokenTag tag);

		bool check(TokenType type)const;
		bool match(TokenType type);
		const Token& consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);

		bool is_at_end()const;

		//当前脚本的类型表
		StructTemplateContainer& current_stc();
		const std::string& current_file_path()const;
		const std::string& current_unit_name()const;
		const ScopeVisitor& current_scope_visitor()const;
		const ModuleVisitor& current_module_visitor()const;

		const ParserDependence* dependence()const;

		Parser* parent_parser_ = nullptr;
	};

	class Parser::ErrorRecoverer :public Parser::ChildParser {
	public:
		ErrorRecoverer(Parser& parent_parser);
		~ErrorRecoverer() = default;
		//恐慌模式跳过错误
		void panic_mode(PanicEnd end);
	private:
		//对于部分符号,直接读到最近的该符号即可
		void panic_mode_common(PanicEnd end);
		//对于),],},要考虑多层嵌套
		void panic_mode_mult(PanicEnd end);
	};

	//用于解析脚本配置的子解析器
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

		//保证子解析器生命周期小于父解析器
		Parser* parent_parser_ = nullptr;
	};

	//使用递归下降法的语法解析器
	class Parser::StatementParser : public Parser::ChildParser {
	public:
		StatementParser(Parser& parent_parser);
		~StatementParser() = default;

		std::unique_ptr<AbstractSyntaxTree> parse_statement();
	private:
		std::unique_ptr<AbstractSyntaxTree> parse_block();
		std::unique_ptr<AbstractSyntaxTree> parse_variable_declaration();
		std::unique_ptr<AbstractSyntaxTree> parse_control_flow();
		std::unique_ptr<AbstractSyntaxTree> parse_jump_flow();
		std::unique_ptr<AbstractSyntaxTree> parse_expression();
	};

	class Parser::ExpressionParser : public Parser::ChildParser {
	public:
		ExpressionParser(Parser& parent_parser);
		~ExpressionParser() = default;

		std::unique_ptr<AbstractSyntaxTree> parse_expression();
	private:
		enum class Precedence {
			None,
			Assign,				// =, +=, -=
			LogicalOr,			// ||
			LogicalAnd,			// &&
			Equality,			// ==, !=
			Comparison,			// <, >, <=, >=
			Additive,			// +, -
			Multiplicative,		// *, /, %
			Unary,				// +, -, ++, --
			Call,				// ()
			Index,				// []
			Member				// .
		};

		enum class Associativity {
			None,
			Left,
			Right
		};

		std::unique_ptr<AbstractSyntaxTree> parse_expression(Precedence precedence);

		std::unique_ptr<AbstractSyntaxTree> parse_primary();
		std::unique_ptr<AbstractSyntaxTree> parse_variable();
		std::unique_ptr<AbstractSyntaxTree> parse_function();
		std::unique_ptr<AbstractSyntaxTree> parse_literal();
		std::unique_ptr<AbstractSyntaxTree> parse_grouping();

		std::unique_ptr<AbstractSyntaxTree> parse_postfix(
			std::unique_ptr<AbstractSyntaxTree> left
		);
		std::unique_ptr<AbstractSyntaxTree> parse_member(
			std::unique_ptr<AbstractSyntaxTree> var
		);
		std::unique_ptr<AbstractSyntaxTree> parse_index(
			std::unique_ptr<AbstractSyntaxTree> var
		);
		std::unique_ptr<AbstractSyntaxTree> parse_function_call(
			std::unique_ptr<AbstractSyntaxTree> callee
		);

		std::unique_ptr<AbstractSyntaxTree> parse_unary();

		std::unique_ptr<AbstractSyntaxTree> parse_initializer_list();


		std::size_t get_precedence_value(TokenType type)const;
		Precedence get_precedence(TokenType type)const;
		Associativity get_associativity(TokenType type)const;
		std::pair<Precedence, Associativity> current_token_attribute()const;

	};

}