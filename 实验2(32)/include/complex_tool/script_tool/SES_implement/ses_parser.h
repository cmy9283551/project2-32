#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

namespace ses {
	//存放脚本配置信息
	struct ScriptConfig {
		//输入输出的参数
		ScriptParameter input, output;

		ModuleVisitor module_visitor;
		ScopeVisitor scope_visitor;
	};

	//存放模组配置信息
	struct ModuleConfig {
		ScopeVisitor scope_visitor;
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
		Parser();
		//用于解析*.ses文件,返回脚本中的所有独立脚本
		virtual std::vector<std::unique_ptr<AbstractSyntaxTree>> parse_ses(
			const std::string& script_path,
			const CompileDependence& dependence
		);
	protected:
		using TokenType = Token::TokenType;

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

	//使用递归下降法的语法解析器
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

	//使用Pratt解析法的语法解析器
	class PrattParser : public Parser {
	protected:
		std::unique_ptr<SESStatementNode> parse_ses_statement()const override;
	};
}