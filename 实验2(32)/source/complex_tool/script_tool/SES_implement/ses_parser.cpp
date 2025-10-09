#include "complex_tool/script_tool/SES_implement/ses_parser.h"

namespace ses {

	Parser::Parser(const CompileDependence& dependence)
		:error_recoerer_(new ErrorRecoverer(*this)),
		config_parser_(new ConfigParser(*this)),
		dependence_(&dependence) {
	}

	std::optional<std::vector<std::unique_ptr<AbstractSyntaxTree>>> Parser::parse_ses(
		const std::string& script_path
	) {
		current_file_path_ = script_path;
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		current_token_stream_ = std::make_unique<TokenStream>(script_path);

		Lexer lexer;
		if (lexer.tokenize(current_token_stream_) == false) {
			return asts;
		}
		while (is_at_end() == false) {
			auto result = parse_ses_script();
			if (result == std::nullopt) {
				//当出现编译失败时,直接结束,避免大量错误数据输入
				return std::nullopt;
			}
			asts.emplace_back(std::move(result.value()));
		}

		current_token_stream_.release();
		return asts;
	}

	Parser::TokenTag Parser::find_tag(TokenType type) const {
		static const std::unordered_map<TokenType, TokenTag> tag_container = {
			//Declaration
			{TokenType::Const,TokenTag::Declaration },
			{TokenType::Int,TokenTag::Declaration },
			{TokenType::Float,TokenTag::Declaration },
			{TokenType::Char,TokenTag::Declaration },
			{TokenType::String,TokenTag::Declaration },
			{TokenType::VectorInt,TokenTag::Declaration },
			{TokenType::VectorFloat,TokenTag::Declaration },
			{TokenType::Package,TokenTag::Declaration }
		};

		auto iter = tag_container.find(type);
		if (iter == tag_container.end()) {
			return TokenTag::Null;
		}
		return iter->second;
	}

	bool Parser::check_tag(TokenTag tag) const {
		return tag == find_tag(current_token().type);
	}

	Parser::IdentifierType Parser::identify() {
		if (current_token().type != TokenType::Identifier) {
			return IdentifierType::Null;
		}
		const std::string& identifier = current_token().value;
		std::size_t size = variable_stack_.size();

		const auto& scope_visitor = current_script_config_->scope_visitor;
		const auto& module_visitor = current_script_config_->module_visitor;
		return IdentifierType::Null;
	}

	Parser::StructTemplateContainer& Parser::current_stc(){
		return current_script_config_->script_stc;
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

	bool Parser::check(const std::vector<TokenType>& type) {
		for (auto value : type) {
			if (check(value) == true) {
				return true;
			}
		}
		return false;
	}

	bool Parser::match(TokenType type) {
		if (check(type) == true) {
			advance();
			return true;
		}
		return false;
	}

	void Parser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		if (check(type) == true) {
			advance();
			return;
		}
		throw ParserErrorMessage(current_token(), message, line, func);
	}

	bool Parser::is_at_end()const {
		return current_token_stream_->is_at_end();
	}

	void Parser::panic_mode_recovery(PanicEnd end) {
		error_recoerer_->panic_mode(end);
	}

	std::optional<std::unique_ptr<AbstractSyntaxTree>> Parser::parse_ses_script() {
		//配置单个脚本参数
		if (check(TokenType::Identifier) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path_, __LINE__, __func__, "Unknown", current_token()
			) << "脚本必须以脚本名开头\n";
			return std::nullopt;
		}
		current_script_name_ = current_token().value;
		advance();//->'['/'{'
		//由于该函数管理配置信息,若跳过会破坏数据,因此在该函数处理异常
		auto handle_error = [this](const ParserErrorMessage& error)
			{
				if (error.show_token) {
					SCRIPT_PARSER_COMPILE_ERROR(
						current_file_path_, error.line, error.func,
						current_script_name_, error.error_token
					) << error.message << " token:" << error.error_token << "\n";
				}
				else {
					SCRIPT_PARSER_COMPILE_ERROR(
						current_file_path_, error.line, error.func,
						current_script_name_, error.error_token
					) << error.message << "\n";
				}
			};

		auto destruct_script = [this]()
			{
				current_script_name_.clear();
				current_script_config_ = nullptr;
				variable_stack_.clear();
			};

		try {
			current_script_config_ = std::move(config_parser_->parse_ses_script_config());
		}
		catch (const ParserErrorMessage& error) {
			destruct_script();
			handle_error(error);
			return std::nullopt;
		}

#ifdef SCRIPT_SES_PARSER_LOG
		SCRIPT_CLOG << *current_script_config_;
#endif // SCRIPT_SES_PARSER_LOG

		//解析脚本内容
		std::unique_ptr<AbstractSyntaxTree> stmt_ptr;
		try { stmt_ptr = parse_ses_statement(); }
		catch (const ParserErrorMessage& error) {
			destruct_script();
			handle_error(error);
			return std::nullopt;
		}
		//清除单个脚本参数
		destruct_script();
		return std::make_unique<ScriptNode>(
			current_script_name_,
			std::move(stmt_ptr),
			std::move(current_script_config_)
		);
	}

	Parser::ErrorRecoverer::ErrorRecoverer(Parser& parent_parser)
		:parent_parser_(&parent_parser) {
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

	Parser::ConfigParser::ConfigParser(Parser& parent_parser)
		:parent_parser_(&parent_parser) {
	}

	std::unique_ptr<ScriptConfig> Parser::ConfigParser::parse_ses_script_config() {
		//start with "["
		std::unique_ptr<ScriptConfig> ptr(new ScriptConfig(
			parent_parser_->dependence_->default_script_config
		));
		if (match(TokenType::LeftBracket) == false) {
			return ptr;
		}
		std::vector<std::string> variable_scope, function_scope, module_list;
		while (check(TokenType::RightBracket) == false && is_at_end() == false) {
			if (check(TokenType::Identifier) == false) {
				SCRIPT_PARSER_THROW_ERROR("脚本配置列表中出现错误符号,而此处需要一个Identifier")
			}
			auto iter = keyword_list_.find(current_token().value);
			if (iter == keyword_list_.end()) {
				SCRIPT_PARSER_THROW_ERROR("不存在的配置选项")
			}
			advance();
			switch (iter->second)
			{
			case Parser::ConfigParser::Keyword::Module:
				parse_module_list(module_list);
				break;
			case Parser::ConfigParser::Keyword::Input:
				parse_parameter(ptr->input);
				break;
			case Parser::ConfigParser::Keyword::OutPut:
				parse_parameter(ptr->output);
				break;
			case Parser::ConfigParser::Keyword::VariableScope:
				parse_variable_scope(variable_scope);
				break;
			case Parser::ConfigParser::Keyword::FunctionScope:
				parse_function_scope(function_scope);
				break;
			default:
				ASSERT(false);
				break;
			}
		}

		analysis(module_list, variable_scope, function_scope, ptr);
		//end with ']'
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

	void Parser::ConfigParser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		parent_parser_->consume(type, message, line, func);
	}

	bool Parser::ConfigParser::is_at_end()const {
		return parent_parser_->is_at_end();
	}

	const std::string& Parser::ConfigParser::current_file_path() const {
		return parent_parser_->current_file_path_;
	}

	const std::string& Parser::ConfigParser::current_script_name() const {
		return parent_parser_->current_script_name_;
	}

	void Parser::ConfigParser::parse_module_list(std::vector<std::string>& module_list) {
		SCRIPT_PARSER_CONSUME(TokenType::LeftBrace, "未找到包含模组列表的{}块")
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
				SCRIPT_PARSER_THROW_ERROR("预期外的符号")
			}
		advance();//skip'}'
	}

	void Parser::ConfigParser::parse_variable_scope(std::vector<std::string>& variable_scope) {
		SCRIPT_PARSER_CONSUME(TokenType::LeftBrace, "未找到包含变量作用域列表的{}块")
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
				SCRIPT_PARSER_THROW_ERROR("预期外的符号")
			}
		advance();//skip'}'
	}

	void Parser::ConfigParser::parse_function_scope(std::vector<std::string>& function_scope) {
		SCRIPT_PARSER_CONSUME(TokenType::LeftBrace, "未找到包含函数作用域列表的{}块")
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
				SCRIPT_PARSER_THROW_ERROR("预期外的符号")
			}
		advance();//skip'}'
	}

	void Parser::ConfigParser::parse_parameter(ScriptParameter& parameter) {
		SCRIPT_PARSER_CONSUME(TokenType::LeftBrace, "未找到包含传入参数列表的{}块")
			auto check_identifier = [this]()
			{
				if (check(TokenType::Identifier) == false) {
					SCRIPT_PARSER_THROW_ERROR("不合语法的参数名称")
				}
			};
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (match(TokenType::Int) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::Int);
				advance();
				continue;
			}
			if (match(TokenType::Float) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::Float);
				advance();
				continue;
			}
			if (match(TokenType::String) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::String);
				advance();
				continue;
			}
			if (match(TokenType::VectorInt) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::VectorInt);
				advance();
				continue;
			}
			if (match(TokenType::VectorFloat) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::VectorFloat);
				advance();
				continue;
			}
			if (match(TokenType::Package) == true) {
				check_identifier();
				parameter.parameters.insert(current_token().value, ParameterType::Package);
				advance();
				continue;
			}
			SCRIPT_PARSER_THROW_ERROR("预期外的符号")
		}
		advance();//skip'}'
	}

	void Parser::ConfigParser::analysis(
		std::vector<std::string>& module_list,
		std::vector<std::string>& variable_scope,
		std::vector<std::string>& function_scope,
		std::unique_ptr<ScriptConfig>& config) const {

		auto scope_result = parent_parser_->dependence_->scope_visitor.init_sub_scope(
			variable_scope, function_scope, config->scope_visitor
		);
		if (scope_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_script_name(), current_token()
			) << "未找到以下作用域\n";
			ScopeVisitor::ScopeNotFound& value = scope_result.value();
			std::size_t size = value.variable_scope.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR_ADDITIONAL
					<< "[variable scope]:" << value.variable_scope[i] << "\n";
			}
			size = value.function_scope.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR_ADDITIONAL
					<< "[function scope]:" << value.function_scope[i] << "\n";
			}
		}

		auto module_result = parent_parser_->dependence_->module_visitor.init_sub_visitor(
			module_list, config->module_visitor
		);
		if (module_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_script_name(), current_token()
			) << "未找到以下模组\n";
			std::size_t size = module_result.value().size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR_ADDITIONAL << "[module]:" << module_result.value()[i] << "\n";
			}
		}

		auto check_result = config->module_visitor.check_scope(config->scope_visitor);
		if (check_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_script_name(), current_token()
			) << "存在无效模组:原因:模组要求配置中不含有的作用域\n";
			const auto& list = check_result.value().invalid_vector;
			std::size_t size = list.size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR_ADDITIONAL
					<< "模组[" << list[i].first << "]要求以下作用域:\n";
				std::size_t s_size = list[i].second.variable_scope.size();
				for (std::size_t j = 0; j < s_size; j++) {
					SCRIPT_COMPILE_ERROR_ADDITIONAL
						<< "[variable scope]:" << list[i].second.variable_scope[i] << "\n";
				}
				s_size = list[i].second.function_scope.size();
				for (std::size_t j = 0; j < s_size; j++) {
					SCRIPT_COMPILE_ERROR_ADDITIONAL
						<< "[function scope]:" << list[i].second.function_scope[i] << "\n";
				}
			}
		}
	}

	RecursiveDescentParser::RecursiveDescentParser(const CompileDependence& dependence)
		:Parser(dependence),
		expression_parser_(std::make_unique<ExpressionParser>(*this)) {
	}

	std::unique_ptr<AbstractSyntaxTree> RecursiveDescentParser::parse_ses_statement() {
		//start with '{'
		SCRIPT_PARSER_CONSUME(TokenType::LeftBrace, "脚本语法块应当包含在{...}之中,但读取到的是")
			std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		while (check(TokenType::LeftBrace) == false) {
			if (match(TokenType::LeftBrace) == true) {
				asts.emplace_back(std::move(parse_block()));
				continue;
			}

			SCRIPT_PARSER_THROW_ERROR("无法识别的句首token")
		}

		return std::make_unique<StmtBlockNode>(asts);
	}

	std::unique_ptr<AbstractSyntaxTree> RecursiveDescentParser::parse_block() {

		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> RecursiveDescentParser::parse_variable_declaration()
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	RecursiveDescentParser::ExpressionParser::ExpressionParser(
		RecursiveDescentParser& parent_parser
	) :parent_parser_(&parent_parser) {
	}

	Token RecursiveDescentParser::ExpressionParser::current_token()const {
		return parent_parser_->current_token();
	}

	void RecursiveDescentParser::ExpressionParser::advance() {
		parent_parser_->advance();
	}

	bool RecursiveDescentParser::ExpressionParser::check(TokenType type)const {
		return parent_parser_->check(type);
	}

	bool RecursiveDescentParser::ExpressionParser::check(
		const std::vector<TokenType>& type
	) const {
		return parent_parser_->check(type);
	}

	bool RecursiveDescentParser::ExpressionParser::match(TokenType type) {
		return parent_parser_->match(type);
	}

	bool RecursiveDescentParser::ExpressionParser::is_at_end()const {
		return parent_parser_->is_at_end();
	}

	void RecursiveDescentParser::ExpressionParser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		parent_parser_->consume(type, message, line, func);
	}

	const std::string& RecursiveDescentParser::ExpressionParser::current_file_path()const {
		return parent_parser_->current_file_path_;
	}

	const std::string& RecursiveDescentParser::ExpressionParser::current_script_name()const {
		return parent_parser_->current_script_name_;
	}

	std::unique_ptr<AbstractSyntaxTree> PrattParser::parse_ses_statement() {

		return std::unique_ptr<AbstractSyntaxTree>();
	}
}