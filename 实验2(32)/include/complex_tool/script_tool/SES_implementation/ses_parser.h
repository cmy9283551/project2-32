﻿#pragma once

#include "ses_lexer.h"
#include "ses_expression.h"

#include <list>
#include <unordered_set>

namespace ses {

#define SCRIPT_SES_PARSER_LOG

#define SCRIPT_PARSER_THROW_ERROR(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__)

#define SCRIPT_PARSER_THROW_ERROR_HIDE_TOKEN(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__,false)

	//用于存储一个代码块中用到的临时变量
	class LocalVariableTable {
	public:
		using StructTemplateContainer = VariableManager::StructTemplateContainer;
		LocalVariableTable() = default;
		LocalVariableTable(StructTemplateContainer& struct_template_container);

		bool push_back(const std::string& type_name, const std::string& var_name);
		bool contains(const std::string& var_name)const;
	private:
		StructTemplateContainer* struct_template_container_;
		//变量名->变量类型代码
		IndexedMap<std::string, std::size_t> variable_table_;
	};

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

			LocalVar,
			ModuleType,
			ModuleFunc,
			InternalType,
			InternalFunc,
			InternalVar,
			Identifier,

			Primary,
			Unary,
			Binary,
			Postfix,

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

		std::optional<TokenTag> identify()const;
		//解析当前identifier的类型
		const TokenTagTable& find_tag(TokenType type)const;
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

		//局部变量表的栈,用于处理变量的作用域
		//以list模拟栈,方便遍历,并允许可以访问非栈顶元素
		std::list<LocalVariableTable> variable_stack_;
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
		Token current_token()const;
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
		Token consume(
			TokenType type,
			const std::string& message,
			std::size_t line, const
			std::string& func
		);

		bool is_at_end()const;

		//当前脚本的类型表
		StructTemplateContainer& current_stc();
		std::list<LocalVariableTable>& current_variable_stack();
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

		//由于表达式解析的解析更复杂,因此应当在每个函数开头检查是否传入能处理的token

		std::unique_ptr<AbstractSyntaxTree> parse_expression(Precedence precedence);

		std::unique_ptr<AbstractSyntaxTree> parse_unary();

		std::unique_ptr<AbstractSyntaxTree> parse_primary();
		std::unique_ptr<AbstractSyntaxTree> parse_variable();
		std::unique_ptr<AbstractSyntaxTree> parse_function();
		std::unique_ptr<AbstractSyntaxTree> parse_literal();
		// Parses expressions enclosed in parentheses.
		std::unique_ptr<AbstractSyntaxTree> parse_grouping();

		std::unique_ptr<AbstractSyntaxTree> parse_postfix(
			std::unique_ptr<AbstractSyntaxTree> left
		);
		std::unique_ptr<AbstractSyntaxTree> parse_function_call(
			std::unique_ptr<AbstractSyntaxTree> callee
		);
		std::unique_ptr<AbstractSyntaxTree> parse_index(
			std::unique_ptr<AbstractSyntaxTree> var
		);
		std::unique_ptr<AbstractSyntaxTree> parse_member(
			std::unique_ptr<AbstractSyntaxTree> var
		);

		std::unique_ptr<AbstractSyntaxTree> parse_binary(
			std::unique_ptr<AbstractSyntaxTree> left,
			Precedence precedence
		);

		std::unique_ptr<AbstractSyntaxTree> parse_initializer_list();


		Precedence token_precedence(TokenType type)const;
		Associativity token_associativity(TokenType type)const;
		std::pair<Precedence, Associativity> current_token_attribute()const;

	};

}