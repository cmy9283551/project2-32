#include "complex_tool/script_tool/SES_implement/ses_parser.h"

namespace ses {
	Parser::Parser()
		:error_recoerer_(new ErrorRecoverer(this)),
		config_parser_(new ConfigParser(this)) {
	}

	std::vector<std::unique_ptr<AbstractSyntaxTree>> Parser::parse_ses(
		const std::string& script_path,
		const CompileDependence& dependence
	) {
		current_file_path_ = script_path;
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		TokenStream token_stream(script_path);
		current_token_stream_ = &token_stream;
		current_dependence_ = &dependence;

		Lexer lexer;
		lexer.tokenize(token_stream);
		while (token_stream.current_token().type != token_stream.end()) {
			auto result = parse_ses_script();
			if (result == nullptr) {
				continue;
			}
			asts.push_back(std::move(result));
		}

		current_token_stream_ = nullptr;
		current_dependence_ = nullptr;
		return asts;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::parse_ses_script() {
		if (check(TokenType::Identifier) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(current_file_path_, "Unknown")
				<< "脚本必须以脚本名开头\n";
			return nullptr;
		}
		current_script_name_ = current_token().value;
		advance();//->'['/'{'
		current_script_config_ = config_parser_->parse_ses_script_config().get();


		//TO DO: 解析脚本内容

		current_script_config_ = nullptr;
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	Token Parser::current_token() const {
		if (current_token_stream_ == nullptr) {
			ASSERT(false);
		}
		return current_token_stream_->current_token();
	}

	void Parser::advance() {
		if (current_token_stream_ == nullptr) {
			ASSERT(false);
		}
		current_token_stream_->advance();
	}

	bool Parser::check(TokenType type) const {
		return current_token_stream_->current_token().type == type;
	}

	bool Parser::match(TokenType type) {
		if (check(type) == true) {
			advance();
			return true;
		}
		return false;
	}

	bool Parser::is_at_end()const {
		return current_token_stream_->is_at_end();
	}

	void Parser::panic_mode_recovery(PanicEnd end) {
		error_recoerer_->panic_mode(end);
	}

	Parser::ErrorRecoverer::ErrorRecoverer(Parser* parent_parser)
		:parent_parser_(parent_parser) {
	}

	void Parser::ErrorRecoverer::panic_mode(PanicEnd end) {
		switch (end)
		{
		case Parser::PanicEnd::RightParen:
			panic_mode_mult(end);
			break;
		case Parser::PanicEnd::RightBracket:
			panic_mode_mult(end);
			break;
		case Parser::PanicEnd::RightBrace:
			panic_mode_mult(end);
			break;
		case Parser::PanicEnd::Semicolon:
			panic_mode_common(end);
			break;
		case Parser::PanicEnd::Comma:
			panic_mode_common(end);
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	void Parser::ErrorRecoverer::panic_mode_common(PanicEnd end) {
		TokenType type;
		switch (end)
		{
		case Parser::PanicEnd::Semicolon:
			type = TokenType::Semicolon;
			break;
		case Parser::PanicEnd::Comma:
			type = TokenType::Comma;
			break;
		default:
			ASSERT(false);
			break;
		}
		while (
			parent_parser_->match(type) == false &&
			parent_parser_->is_at_end() == false
			) {
		}
	}

	void Parser::ErrorRecoverer::panic_mode_mult(PanicEnd end) {
		TokenType left, right;
		switch (end)
		{
		case Parser::PanicEnd::RightParen:
			left = TokenType::LeftParen;
			right = TokenType::RightParen;
			break;
		case Parser::PanicEnd::RightBracket:
			left = TokenType::LeftBracket;
			right = TokenType::RightBracket;
			break;
		case Parser::PanicEnd::RightBrace:
			left = TokenType::LeftBrace;
			right = TokenType::RightBrace;
			break;
		default:
			ASSERT(false);
			break;
		}
		std::size_t count = 1;
		while (count != 0 && parent_parser_->is_at_end() == false) {
			if (parent_parser_->check(left)) {
				count++;
			}
			if (parent_parser_->check(right)) {
				count--;
			}
			parent_parser_->advance();
		}
	}

	const std::unordered_map<std::string, Parser::ConfigParser::Keyword>
		Parser::ConfigParser::keyword_list_ = {
			{"module",Parser::ConfigParser::Keyword::Module},
			{"parameter",Parser::ConfigParser::Keyword::Input},
			{"return_value",Parser::ConfigParser::Keyword::OutPut},
			{"variable_scope",Parser::ConfigParser::Keyword::VariableScope},
			{"function_scope",Parser::ConfigParser::Keyword::FunctionScope}
	};

	Parser::ConfigParser::ConfigParser(Parser* parent_parser)
		:parent_parser_(parent_parser) {
	}

	std::shared_ptr<ScriptConfig> Parser::ConfigParser::parse_ses_script_config() {
		std::shared_ptr<ScriptConfig> ptr(new ScriptConfig(
			parent_parser_->current_dependence_->default_script_config
		));
		if (match(TokenType::LeftBracket) == false) {
			return ptr;
		}
		std::vector<std::string> variable_scope, function_scope, module_list;
		while (check(TokenType::RightBracket) == false && is_at_end() == false) {
			if (match(TokenType::Identifier) == false) {
				SCRIPT_PARSER_COMPILE_ERROR(
					current_file_path(), current_script_name()
				) << "脚本配置列表中出现错误符号" << current_token()
					<< ",而此处需要一个Identifier\n";
				panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			auto iter = keyword_list_.find(current_token().value);
			if (iter == keyword_list_.end()) {
				SCRIPT_PARSER_COMPILE_ERROR(
					current_file_path(), current_script_name()
				) << "不存在配置选项[" << current_token().value << "]\n";
				panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			switch (iter->second)
			{
			case Parser::ConfigParser::Keyword::Module:
				if (parse_module_list(module_list) == false) {
					panic_mode_recovery(PanicEnd::RightBracket);
					return ptr;
				}
				break;
			case Parser::ConfigParser::Keyword::Input:
				if (parse_parameter(ptr->input) == false) {
					panic_mode_recovery(PanicEnd::RightBracket);
					return ptr;
				}
				break;
			case Parser::ConfigParser::Keyword::OutPut:
				if (parse_parameter(ptr->output) == false) {
					panic_mode_recovery(PanicEnd::RightBracket);
					return ptr;
				}
				break;
			case Parser::ConfigParser::Keyword::VariableScope:
				if (parse_variable_scope(variable_scope) == false) {
					panic_mode_recovery(PanicEnd::RightBracket);
					return ptr;
				}
				break;
			case Parser::ConfigParser::Keyword::FunctionScope:
				if (parse_function_scope(function_scope) == false) {
					panic_mode_recovery(PanicEnd::RightBracket);
					return ptr;
				}
				break;
			default:
				ASSERT(false);
				break;
			}
		}

		analysis(module_list, variable_scope, function_scope, ptr);

		advance();//skip']'
		return ptr;
	}

	Token Parser::ConfigParser::current_token()const {
		return parent_parser_->current_token();
	}

	void Parser::ConfigParser::advance() {
		parent_parser_->advance();
	}

	bool Parser::ConfigParser::check(TokenType type)const {
		return parent_parser_->check(type);
	}

	bool Parser::ConfigParser::match(TokenType type) {
		return parent_parser_->match(type);
	}

	bool Parser::ConfigParser::is_at_end()const {
		return parent_parser_->is_at_end();
	}

	void Parser::ConfigParser::panic_mode_recovery(PanicEnd end) {
		parent_parser_->panic_mode_recovery(end);
	}

	const std::string& Parser::ConfigParser::current_file_path() const {
		return parent_parser_->current_file_path_;
	}

	const std::string& Parser::ConfigParser::current_script_name() const {
		return parent_parser_->current_script_name_;
	}

	bool Parser::ConfigParser::parse_module_list(std::vector<std::string>& module_list) {
		if (match(TokenType::LeftBrace) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到包含模组列表的{}块\n";
			return false;
		}
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (check(TokenType::Identifier) == true) {
				module_list.push_back(current_token().value);
				advance();
				continue;
			}
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "预期外的符号" << current_token() << "\n";
			return false;
		}
		return true;
	}

	bool Parser::ConfigParser::parse_variable_scope(std::vector<std::string>& variable_scope) {
		if (match(TokenType::LeftBrace) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到包含变量作用域列表的{}块\n";
			return false;
		}
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (check(TokenType::Identifier) == true) {
				variable_scope.push_back(current_token().value);
				advance();
				continue;
			}
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "预期外的符号" << current_token() << "\n";
			return false;
		}
		return true;
	}

	bool Parser::ConfigParser::parse_function_scope(std::vector<std::string>& function_scope) {
		if (match(TokenType::LeftBrace) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到包含函数作用域列表的{}块\n";
			return false;
		}
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (check(TokenType::Identifier) == true) {
				function_scope.push_back(current_token().value);
				advance();
				continue;
			}
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "预期外的符号" << current_token() << "\n";
			return false;
		}
		return true;
	}

	bool Parser::ConfigParser::parse_parameter(ScriptParameter& parameter) {
		if (match(TokenType::LeftBrace) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到包含传入参数列表的{}块\n";
			return false;
		}
		auto check_identifier = [this]()->bool
			{
				if (check(TokenType::Identifier) == false) {
					SCRIPT_PARSER_COMPILE_ERROR(
						current_file_path(), current_script_name()
					) << "不合语法的参数名称" << current_token() << "\n";
					return false;
				}
				return true;
			};
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (match(TokenType::Int) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::Int);
				advance();
				continue;
			}
			if (match(TokenType::Float) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::Float);
				advance();
				continue;
			}
			if (match(TokenType::String) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::String);
				advance();
				continue;
			}
			if (match(TokenType::VectorInt) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::VectorInt);
				advance();
				continue;
			}
			if (match(TokenType::VectorFloat) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::VectorFloat);
				advance();
				continue;
			}
			if (match(TokenType::Package) == true) {
				if (check_identifier() == false) {
					return false;
				}
				parameter.parameter_list.emplace(current_token().value, ParameterType::Package);
				advance();
				continue;
			}
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "预期外的符号" << current_token() << "\n";
			return false;
		}
		return true;
	}

	void Parser::ConfigParser::analysis(
		std::vector<std::string>& module_list,
		std::vector<std::string>& variable_scope,
		std::vector<std::string>& function_scope,
		std::shared_ptr<ScriptConfig> config) const {

		auto scope_result = parent_parser_->current_dependence_->scope_visitor.init_sub_scope(
			variable_scope, function_scope, config->scope_visitor
		);
		if (scope_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到以下作用域\n";
			ScopeVisitor::ScopeNotFound& value = scope_result.value();
			std::size_t size = value.variable_scope.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR
					<< "[variable scope]:" << value.variable_scope[i] << "\n";
			}
			size = value.function_scope.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR
					<< "[function scope]:" << value.function_scope[i] << "\n";
			}
		}

		auto module_result = parent_parser_->current_dependence_->module_visitor.init_sub_visitor(
			module_list, config->module_visitor
		);
		if (module_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "未找到以下模组\n";
			std::size_t size = module_result.value().size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR << "[module]:" << module_result.value()[i] << "\n";
			}
		}

		auto check_result = config->module_visitor.check_scope(config->scope_visitor);
		if (check_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path(), current_script_name()
			) << "存在无效模组:原因:模组要求配置中不含有的作用域\n";
			const auto& list = check_result.value().invalid_list;
			std::size_t size = list.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR
					<< "模组[" << list[i].first << "]要求以下作用域:\n";
				std::size_t s_size = list[i].second.variable_scope.size();
				for (std::size_t j = 0; j < s_size; j++) {
					SCRIPT_COMPILE_ERROR
						<< "[variable scope]:" << list[i].second.variable_scope[i] << "\n";
				}
				s_size = list[i].second.function_scope.size();
				for (std::size_t j = 0; j < s_size; j++) {
					SCRIPT_COMPILE_ERROR
						<< "[function scope]:" << list[i].second.function_scope[i] << "\n";
				}
			}
		}
	}

	std::unique_ptr<StatementNode> RecursiveDescentParser::parse_ses_statement()const {

		return std::unique_ptr<StatementNode>();
	}

	std::unique_ptr<StatementNode> PrattParser::parse_ses_statement()const {

		return std::unique_ptr<StatementNode>();
	}
}