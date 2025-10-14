#include "complex_tool/script_tool/SES_implementation/ses_parser.h"

namespace ses {

#ifdef  SCRIPT_SHOW_ERROR_LOCATION

#define SCRIPT_PARSER_COMPILE_WARNING(script_file,script_name,token)\
std::clog<<"[Script Warning](parser)"<<\
"(file:"<<__FILE__<<")\n(line:"<<__LINE__<<")("<<__func__<<")\n"\
<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#define SCRIPT_PARSER_COMPILE_ERROR(script_file,error_line,func,script_name,token)\
std::clog<<"[Script Warning](parser)"<<\
"(file:"<<__FILE__<<")\n(line:"<<error_line<<")("<<func<<")\n"\
<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#else

#define SCRIPT_PARSER_COMPILE_WARNING(script_file,script_name,token)\
std::clog<<"[Script Warning](parser)"<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#define SCRIPT_PARSER_COMPILE_ERROR(script_file,error_line,func,script_name,token)\
std::clog<<"[Script Error](parser)"<<"(script file:"<<script_file<<"):\n"\
<<"(script name:"<<script_name<<")(line "<<token.line<<"):\n"

#endif //  SCRIPT_SHOW_ERROR_LOCATION

#define SCRIPT_PARSER_THROW_ERROR(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__)

#define SCRIPT_PARSER_THROW_ERROR_HIDE_TOKEN(message)\
	throw ParserErrorMessage(current_token(),message,__LINE__,__func__,false)

	Parser::Parser(const ParserDependence& dependence)
		:error_recoerer_(std::make_unique<ErrorRecoverer>(*this)),
		statement_parser_(std::make_unique<StatementParser>(*this)),
		expression_parser_(std::make_unique<ExpressionParser>(*this)),
		dependence_(&dependence) {
	}

	std::optional<std::vector<std::unique_ptr<AbstractSyntaxTree>>> Parser::parse(
		const std::string& script_path
	) {
		current_file_path_ = script_path;
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		current_token_stream_ = std::make_unique<TokenStream>(script_path);

		Lexer lexer;
		if (lexer.tokenize(current_token_stream_) == false) {
			return std::nullopt;
		}
		std::cout << *current_token_stream_ << std::endl;
		while (is_at_end() == false) {
			auto result = parse_unit();
			if (result == std::nullopt) {
				//当出现编译失败时,直接结束,避免大量错误数据输入
				return std::nullopt;
			}
			asts.emplace_back(std::move(result.value()));
		}

		current_token_stream_.release();
		return asts;
	}

	bool Parser::is_type_name()const {
		if (current_token().type != TokenType::Identifier) {
			return false;
		}
		const std::string& identifier = current_token().value;
		//保证scope_visitor和module_visitor无名称冲突
		const auto& scope_visitor = current_scope_visitor();
		auto sv_result = scope_visitor.identify(identifier);
		if (sv_result.has_value() == true) {
			if (sv_result.value() == ScopeVisitor::IdentifierType::TypeName) {
				return true;
			}
		}
		const auto& module_visitor = current_module_visitor();
		auto mv_result = module_visitor.identify(identifier);
		if (mv_result.has_value() == true) {
			if (mv_result.value() == Module::IdentifierType::TypeName) {
				return true;
			}
		}
		return false;
	}

	const Parser::TokenTagTable& Parser::find_tag(TokenType type) const {
		static const std::unordered_map<TokenType, TokenTagTable> tag_container = {
			{TokenType::Const,{TokenTag::Const} },
			//TypeName
			{TokenType::Int,{TokenTag::TypeName} },
			{TokenType::Float,{TokenTag::TypeName} },
			{TokenType::Char,{TokenTag::TypeName} },
			{TokenType::String,{TokenTag::TypeName} },
			{TokenType::VectorInt,{TokenTag::TypeName} },
			{TokenType::VectorFloat,{TokenTag::TypeName} },
			{TokenType::Package,{TokenTag::TypeName} },
			//ControlFlow
			{TokenType::If,{TokenTag::ControlFlow} },
			{TokenType::Else,{TokenTag::ControlFlow} },
			{TokenType::While,{TokenTag::ControlFlow} },
			{TokenType::For,{TokenTag::ControlFlow} },
			//JumpFlow
			{TokenType::Return,{TokenTag::JumpFlow} },
			{TokenType::Break,{TokenTag::JumpFlow} },
			{TokenType::Continue,{TokenTag::JumpFlow} },
			//Literal
			{TokenType::LiteralInt,{TokenTag::Literal,TokenTag::Primary} },
			{TokenType::LiteralFloat,{TokenTag::Literal,TokenTag::Primary} },
			{TokenType::LiteralChar,{TokenTag::Literal,TokenTag::Primary} },
			{TokenType::LiteralString,{TokenTag::Literal,TokenTag::Primary} },
			{TokenType::LiteralBool,{TokenTag::Literal,TokenTag::Primary} },
			//Identifier
			{TokenType::Identifier,{TokenTag::Identifier,TokenTag::Primary} },
			//Unary
			{TokenType::Minus,{TokenTag::Unary,TokenTag::Binary} },
			{TokenType::Plus,{TokenTag::Unary,TokenTag::Binary} },
			{TokenType::LogicalNot,{TokenTag::Unary} },
			//Binary
			{TokenType::Assign,{TokenTag::Binary} },
			{TokenType::PlusAssign,{TokenTag::Binary} },
			{TokenType::MinusAssign,{TokenTag::Binary} },
			{TokenType::MultiplyAssign,{TokenTag::Binary} },
			{TokenType::DivideAssign,{TokenTag::Binary} },
			{TokenType::ModuloAssign,{TokenTag::Binary} },
			{TokenType::Multiply,{TokenTag::Binary} },
			{TokenType::Divide,{TokenTag::Binary} },
			{TokenType::Modulo,{TokenTag::Binary} },
			//Delimiter
			{TokenType::LeftParen,{TokenTag::Primary,TokenTag::Postfix} },
			{TokenType::RightParen,{ } },
			{TokenType::LeftBracket,{TokenTag::Postfix} },
			{TokenType::RightBracket,{ } },
			{TokenType::LeftBrace,{ } },
			{TokenType::RightBrace,{ } },
			{TokenType::Semicolon,{ } },
			{TokenType::Comma,{ } },
			{TokenType::Dot,{TokenTag::Postfix} },
		};
		static const TokenTagTable complex_type = {
			TokenTag::TypeName
		};
		static const TokenTagTable no_tag = {
			TokenTag::NoTag
		};

		auto iter = tag_container.find(type);
		if (iter->second.contains(TokenTag::Identifier)) {
			if (is_type_name() == true) {
				return complex_type;
			}
			return iter->second;
		}
		if (iter != tag_container.end()) {
			return iter->second;
		}
		return no_tag;
	}

	bool Parser::check_tag(TokenTag tag) const {
		return find_tag(current_token().type).contains(tag);
	}

	bool Parser::check_tag(const std::vector<TokenTag>& tag) const {
		for (auto value : tag) {
			if (check_tag(value) == true) {
				return true;
			}
		}
		return false;
	}

	bool Parser::match_tag(TokenTag tag) {
		if (check_tag(tag) == true) {
			advance();
			return true;
		}
		return false;
	}

	const Token& Parser::current_token() const {
		if (current_token_stream_ == nullptr) {
			ASSERT(false);
		}
		return current_token_stream_->current_token();
	}

	const Token& Parser::look_ahead(std::size_t step) const {
		if (current_token_stream_ == nullptr) {
			ASSERT(false);
		}
		return current_token_stream_->look_ahead(step);
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

	const Token& Parser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		const auto& token = current_token();
		if (check(type) == true) {
			advance();
			return token;
		}
		throw ParserErrorMessage(current_token(), message, line, func);
	}

	bool Parser::is_at_end()const {
		return current_token_stream_->is_at_end();
	}

	void Parser::panic_mode_recovery(PanicEnd end) {
		error_recoerer_->panic_mode(end);
	}

	ScriptParser::ScriptParser(const ParserDependence& dependence)
		:Parser(dependence), script_config_parser_(std::make_unique<ScriptConfigParser>(*this)) {
	}

	Parser::StructTemplateContainer& ScriptParser::current_stc() {
		return current_script_config_->script_stc;
	}

	const std::string& ScriptParser::current_unit_name() const {
		return current_script_name_;
	}

	const ScopeVisitor& ScriptParser::current_scope_visitor() const {
		return current_script_config_->scope_visitor;
	}

	const ModuleVisitor& ScriptParser::current_module_visitor() const {
		return current_script_config_->module_visitor;
	}

	std::optional<std::unique_ptr<AbstractSyntaxTree>> ScriptParser::parse_unit() {
		//配置单个脚本参数
		if (check(TokenType::Identifier) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				current_file_path_, __LINE__, __func__, "Unknown", current_token()
			) << "脚本必须以脚本名开头\n";
			return std::nullopt;
		}
		current_script_name_ = current_token().value;
		std::size_t script_line = current_token().line;
		//备份脚本名,因为AST在清空当前信息后创建
		std::string script_name = current_script_name_;
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
			};

		try {
			current_script_config_ = std::move(script_config_parser_->parse_ses_script_config());
		}
		catch (const ParserErrorMessage& error) {
			handle_error(error);
			destruct_script();
			return std::nullopt;
		}

#ifdef SCRIPT_SES_PARSER_LOG
		SCRIPT_CLOG << *current_script_config_;
#endif // SCRIPT_SES_PARSER_LOG

		//解析脚本内容
		std::unique_ptr<AbstractSyntaxTree> stmt_ptr;
		try { stmt_ptr = std::move(statement_parser_->parse_statement()); }
		catch (const ParserErrorMessage& error) {
			handle_error(error);
			destruct_script();
			return std::nullopt;
		}
		//清除单个脚本参数
		destruct_script();
		return std::make_unique<ScriptNode>(
			SourceLocation(script_name, script_line),
			current_script_name_,
			std::move(stmt_ptr),
			std::move(current_script_config_)
		);
	}

	Parser::ChildParser::ChildParser(Parser& parent_parser)
		:parent_parser_(&parent_parser) {
	}

	const Token& Parser::ChildParser::current_token()const {
		return parent_parser_->current_token();
	}

	const Token& Parser::ChildParser::look_ahead(std::size_t step)const {
		return parent_parser_->look_ahead(step);
	}

	void Parser::ChildParser::advance() {
		parent_parser_->advance();
	}

	Parser::TokenTag Parser::ChildParser::find_tag(
		TokenType type, const std::vector<TokenTag>& scope
	) const {
		std::size_t size = scope.size();
		TokenTag result = TokenTag::NoTag;
		for (std::size_t i = 0; i < size; i++) {
			if (parent_parser_->check_tag(scope[i]) == false) {
				continue;
			}
			if (result == TokenTag::NoTag) {
				result = scope[i];
			}
			else {
				//此处的MoreThanOneTag是相对于传入的scope而言的
				//并非表示该token有多个tag
				return TokenTag::MoreThanOneTag;
			}

		}
		return result;
	}

	bool Parser::ChildParser::check_tag(TokenTag tag)const {
		return parent_parser_->check_tag(tag);
	}

	bool Parser::ChildParser::check_tag(const std::vector<TokenTag>& tag)const {
		return parent_parser_->check_tag(tag);
	}

	bool Parser::ChildParser::match_tag(TokenTag tag) {
		return parent_parser_->match_tag(tag);
	}

	bool Parser::ChildParser::check(TokenType type)const {
		return parent_parser_->check(type);
	}

	bool Parser::ChildParser::match(TokenType type) {
		return parent_parser_->match(type);
	}

	const Token& Parser::ChildParser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		return parent_parser_->consume(type, message, line, func);
	}

	bool Parser::ChildParser::is_at_end()const {
		return parent_parser_->is_at_end();
	}

	Parser::StructTemplateContainer& Parser::ChildParser::current_stc() {
		return parent_parser_->current_stc();
	}

	const std::string& Parser::ChildParser::current_file_path() const {
		return parent_parser_->current_file_path_;
	}

	const std::string& Parser::ChildParser::current_unit_name() const {
		return parent_parser_->current_unit_name();
	}

	const ScopeVisitor& Parser::ChildParser::current_scope_visitor() const {
		return parent_parser_->current_scope_visitor();
	}

	const ModuleVisitor& Parser::ChildParser::current_module_visitor() const {
		return parent_parser_->current_module_visitor();
	}

	const ParserDependence* Parser::ChildParser::dependence() const {
		return parent_parser_->dependence_;
	}

	Parser::ErrorRecoverer::ErrorRecoverer(Parser& parent_parser)
		:ChildParser(parent_parser) {
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
			SCRIPT_PARSER_THROW_ERROR("Logical Error");
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
			SCRIPT_PARSER_THROW_ERROR("Logical Error");
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
			SCRIPT_PARSER_THROW_ERROR("Logical Error");
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

	const std::unordered_map<std::string, ScriptParser::ScriptConfigParser::Keyword>
		ScriptParser::ScriptConfigParser::keyword_list_ = {
			{"module",ScriptParser::ScriptConfigParser::Keyword::Module},
			{"parameter",ScriptParser::ScriptConfigParser::Keyword::Input},
			{"return_value",ScriptParser::ScriptConfigParser::Keyword::OutPut},
			{"variable_scope",ScriptParser::ScriptConfigParser::Keyword::VariableScope},
			{"function_scope",ScriptParser::ScriptConfigParser::Keyword::FunctionScope}
	};

	ScriptParser::ScriptConfigParser::ScriptConfigParser(Parser& parent_parser)
		:ChildParser(parent_parser) {
	}

	std::unique_ptr<ScriptConfig> ScriptParser::ScriptConfigParser::parse_ses_script_config() {
		//start with "["
		std::unique_ptr<ScriptConfig> ptr(new ScriptConfig(
			*dependence()->default_script_config
		));
		if (match(TokenType::LeftBracket) == false) {
			return ptr;
		}
		std::vector<std::string> variable_scope, function_scope, module_list;
		while (check(TokenType::RightBracket) == false && is_at_end() == false) {
			Token option_token = consume(
				TokenType::Identifier, "脚本配置列表中出现错误符号,而此处需要一个Identifier",
				__LINE__, __func__
			);
			auto iter = keyword_list_.find(option_token.value);
			if (iter == keyword_list_.end()) {
				SCRIPT_PARSER_THROW_ERROR("不存在的配置选项");
			}
			switch (iter->second)
			{
			case ScriptParser::ScriptConfigParser::Keyword::Module:
				parse_module_list(module_list);
				break;
			case ScriptParser::ScriptConfigParser::Keyword::Input:
				parse_parameter(ptr->input);
				break;
			case ScriptParser::ScriptConfigParser::Keyword::OutPut:
				parse_parameter(ptr->output);
				break;
			case ScriptParser::ScriptConfigParser::Keyword::VariableScope:
				parse_variable_scope(variable_scope);
				break;
			case ScriptParser::ScriptConfigParser::Keyword::FunctionScope:
				parse_function_scope(function_scope);
				break;
			default:
				SCRIPT_PARSER_THROW_ERROR("Logical Error");
				break;
			}
		}

		analysis(module_list, variable_scope, function_scope, ptr);
		//end with ']'
		advance();//skip']'
		return ptr;
	}

	void ScriptParser::ScriptConfigParser::parse_module_list(std::vector<std::string>& module_list) {
		consume(TokenType::LeftBrace, "未找到包含模组列表的{}块", __LINE__, __func__);
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号");
		}
		advance();//skip'}'
	}

	void ScriptParser::ScriptConfigParser::parse_variable_scope(std::vector<std::string>& variable_scope) {
		consume(TokenType::LeftBrace, "未找到包含变量作用域列表的{}块", __LINE__, __func__);
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号");
		}
		advance();//skip'}'
	}

	void ScriptParser::ScriptConfigParser::parse_function_scope(std::vector<std::string>& function_scope) {
		consume(TokenType::LeftBrace, "未找到包含函数作用域列表的{}块", __LINE__, __func__);
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号");
		}
		advance();//skip'}'
	}

	void ScriptParser::ScriptConfigParser::parse_parameter(ScriptParameter& parameter) {
		consume(TokenType::LeftBrace, "未找到包含传入参数列表的{}块", __LINE__, __func__);
		while (check(TokenType::RightBrace) == false && is_at_end() == false) {
			if (check(TokenType::Comma) == true) {
				advance();
				continue;
			}
			if (match(TokenType::Int) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::Int);
				continue;
			}
			if (match(TokenType::Float) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::Float);
				continue;
			}
			if (match(TokenType::String) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::String);
				continue;
			}
			if (match(TokenType::VectorInt) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::VectorInt);
				continue;
			}
			if (match(TokenType::VectorFloat) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::VectorFloat);
				continue;
			}
			if (match(TokenType::Package) == true) {
				Token identifier_token = consume(
					TokenType::Identifier, "参数列表中类型后应当跟随参数名",
					__LINE__, __func__
				);
				parameter.parameters.insert(identifier_token.value, ParameterType::Package);
				continue;
			}
			SCRIPT_PARSER_THROW_ERROR("预期外的符号");
		}
		advance();//skip'}'
	}

	void ScriptParser::ScriptConfigParser::analysis(
		std::vector<std::string>& module_list,
		std::vector<std::string>& variable_scope,
		std::vector<std::string>& function_scope,
		std::unique_ptr<ScriptConfig>& config) const {

		auto scope_result = dependence()->scope_visitor->init_sub_scope(
			variable_scope, function_scope, config->scope_visitor
		);
		if (scope_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_unit_name(), current_token()
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

		auto module_result = dependence()->module_visitor->init_sub_visitor(
			module_list, config->module_visitor
		);
		if (module_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_unit_name(), current_token()
			) << "未找到以下模组\n";
			std::size_t size = module_result.value().size();
			for (std::size_t i = 0; i < size; i++) {
				SCRIPT_COMPILE_ERROR_ADDITIONAL << "[module]:" << module_result.value()[i] << "\n";
			}
		}

		auto check_result = config->module_visitor.check_scope(config->scope_visitor);
		if (check_result != std::nullopt) {
			SCRIPT_PARSER_COMPILE_WARNING(
				current_file_path(), current_unit_name(), current_token()
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

	Parser::StatementParser::StatementParser(Parser& parent_parser)
		: ChildParser(parent_parser) {
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_statement() {
		return parse_block();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_block() {
		//start with '{'
		std::size_t script_line = current_token().line;
		consume(
			TokenType::LeftBrace, "脚本语法块应当包含在{...}之中,但读取到的是", __LINE__, __func__
		);
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		while (check(TokenType::RightBrace) == false) {
			if (check(TokenType::LeftBrace) == true) {
				asts.emplace_back(std::move(parse_block()));
				continue;
			}
			if (check_tag({
				TokenTag::TypeName,TokenTag::Const
				}) == true) {
				asts.emplace_back(std::move(parse_variable_declaration()));
				continue;
			}
			if (check_tag({
				TokenTag::Primary,TokenTag::Unary
				}) == true) {
				asts.emplace_back(std::move(parse_expression()));
				continue;
			}
			if (check_tag(TokenTag::ControlFlow) == true) {
				asts.emplace_back(std::move(parse_control_flow()));
				continue;
			}
			if (check_tag(TokenTag::JumpFlow) == true) {
				asts.emplace_back(std::move(parse_jump_flow()));
				continue;
			}
			if (match(TokenType::Semicolon) == true) {
				continue;
			}
			SCRIPT_PARSER_THROW_ERROR("无法识别的句首token");
		}
		//end with '}'
		return std::make_unique<StmtBlockNode>(
			SourceLocation(current_unit_name(), script_line), asts
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_variable_declaration() {
		//start with TypeName/InternalType/ModuleType/Const
		std::size_t script_line = current_token().line;
		TokenTag tag = find_tag(current_token().type,
			{ TokenTag::TypeName,TokenTag::Const }
		);
		if (tag == TokenTag::NoTag || tag == TokenTag::MoreThanOneTag) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的变量声明语句");
		}

		auto handle_type = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token type_name = current_token();
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"变量声明语句中缺少变量名称",
					__LINE__, __func__
				);
				if (match(TokenType::Assign) == true) {
					return std::make_unique<StmtDeclarationNode>(
						SourceLocation(current_unit_name(), script_line),
						type_name.value, var_name.value,
						parent_parser_->statement_parser_->parse_expression()
					);
				}
				return std::make_unique<StmtDeclarationNode>(
					SourceLocation(current_unit_name(), script_line),
					type_name.value, var_name.value
				);
			};
		auto handle_const = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'const'
				if (check_tag({ TokenTag::TypeName }) == false) {
					SCRIPT_PARSER_THROW_ERROR("const后应当跟随类型");
				}
				Token type_name = current_token();
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"常量声明语句中缺少常量名称",
					__LINE__, __func__
				);
				if (match(TokenType::Assign) == true) {
					return std::make_unique<StmtDeclarationNode>(
						SourceLocation(current_unit_name(), script_line),
						type_name.value, var_name.value,
						parent_parser_->statement_parser_->parse_expression(),
						true
					);
				}
				return std::make_unique<StmtDeclarationNode>(
					SourceLocation(current_unit_name(), script_line),
					type_name.value, var_name.value, nullptr, true
				);
			};

		switch (tag) {
		case TokenTag::TypeName:
			return handle_type();
		case TokenTag::Const:
			return handle_const();
		default:
			break;
		}
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_control_flow() {
		//start with ControlFlow(If,Else,While,For)
		std::size_t script_line = current_token().line;
		TokenType type = current_token().type;

		auto handle_if = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'if'
				consume(TokenType::LeftParen, "if语句中缺少'('", __LINE__, __func__);
				auto condition = parse_expression();
				consume(TokenType::RightParen, "if语句中缺少')'", __LINE__, __func__);
				auto then_branch = parse_block();
				std::unique_ptr<AbstractSyntaxTree> else_branch = nullptr;
				if (match(TokenType::Else) == true) {
					else_branch = parse_block();
				}
				return std::make_unique<StmtIfNode>(
					SourceLocation(current_unit_name(), script_line),
					std::move(condition),
					std::move(then_branch),
					std::move(else_branch)
				);
			};

		auto handle_while = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'while'
				consume(TokenType::LeftParen, "while语句中缺少'('", __LINE__, __func__);
				auto condition = parse_expression();
				consume(TokenType::RightParen, "while语句中缺少')'", __LINE__, __func__);
				auto body = parse_block();
				return std::make_unique<StmtWhileNode>(
					SourceLocation(current_unit_name(), script_line),
					std::move(condition),
					std::move(body)
				);
			};

		auto handle_for = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				SCRIPT_PARSER_THROW_ERROR("暂不支持for语句");
				return nullptr;
			};

		switch (type) {
		case TokenType::If:
			return handle_if();
		case TokenType::While:
			return handle_while();
		case TokenType::For:
			return handle_for();
		case TokenType::Else:
			SCRIPT_PARSER_THROW_ERROR("else语句缺少对应的if语句");
			break;
		default:
			break;
		}
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_jump_flow() {
		//start with JumpFlow(Return,Break,Continue)
		std::size_t script_line = current_token().line;
		TokenType type = current_token().type;

		auto handle_continue = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'continue'
				consume(TokenType::Semicolon, "continue语句缺少结尾的';'", __LINE__, __func__);
				return std::make_unique<StmtContinueNode>(
					SourceLocation(current_unit_name(), script_line)
				);
			};

		auto handle_break = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'break'
				consume(TokenType::Semicolon, "break语句缺少结尾的';'", __LINE__, __func__);
				return std::make_unique<StmtBreakNode>(
					SourceLocation(current_unit_name(), script_line)
				);
			};

		auto handle_return = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'return'
				std::unique_ptr<AbstractSyntaxTree> value = nullptr;
				if (check(TokenType::Semicolon) == false) {
					value = parse_expression();
				}
				consume(TokenType::Semicolon, "return语句缺少结尾的';'", __LINE__, __func__);
				return std::make_unique<StmtReturnNode>(
					SourceLocation(current_unit_name(), script_line),
					std::move(value)
				);
			};

		switch (type) {
		case TokenType::Continue:
			return handle_continue();
		case TokenType::Break:
			return handle_break();
		case TokenType::Return:
			return handle_return();
		default:
			break;
		}
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_expression() {
		return std::make_unique<StmtExpressionNode>(
			SourceLocation(current_unit_name(), current_token().line),
			parent_parser_->expression_parser_->parse_expression()
		);
	}

	Parser::ExpressionParser::ExpressionParser(Parser& parent_parser)
		:ChildParser(parent_parser) {
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_expression() {
		//start with Unary or Primary
		return parse_expression(Precedence::Assign);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_expression(
		Precedence precedence
	) {
		//start with Unary or Primary
		if (check_tag({ TokenTag::Unary,TokenTag::Primary }) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		}
		auto left = parse_unary();
		TO_DO_ASSERT;
		//TO DO:处理后缀表达式(函数调用,数组下标,成员访问)
		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_primary() {
		//start with Primary
		//(Literal,Variable,Function,LeftParen)
		if (check_tag(TokenTag::Primary) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		}
		//后缀处理让子函数处理,因此这里不处理后缀,使子函数拥有自行决定实现的权利
		if (check_tag(TokenTag::Literal) == true) {
			return parse_literal();
		}
		if (check(TokenType::LeftParen) == true) {
			return parse_grouping();
		}
		if (check(TokenType::Identifier) == true) {
			//需要区分变量和函数
			//通过查看下一个token是否为'('来区分
			if (look_ahead().type == TokenType::LeftParen) {
				return parse_function();
			}
			else {
				return parse_variable();
			}
		}
		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_variable() {
		//start with Identifier
		if (check(TokenType::Identifier) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的变量");
		}
		Token var_token = current_token();
		advance();
		//变量需要处理后缀表达式
		return parse_postfix(
			std::make_unique<ExprVariableNode>(
				SourceLocation(current_unit_name(), var_token.line),
				var_token.value
			)
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_function() {
		//start with Identifier
		if (check(TokenType::Identifier) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的函数");
		}
		Token func_token = current_token();
		advance();//-> to '('
		//这里的函数只能是通过Identifier识别的函数,
		//因此生成的节点只能是ExprFuncNode,传入函数名,
		//应当与在后缀处理中处理函数调用区分开来
		return parse_postfix(
			std::make_unique<ExprFuncNode>(
				SourceLocation(current_unit_name(), func_token.line),
				func_token.value
			)
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_literal() {
		//start with Literal
		if (check_tag(TokenTag::Literal) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的字面量");
		}

		TokenType type = current_token().type;
		std::size_t script_line = current_token().line;

		auto handle_int = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token int_token = current_token();
				advance();
				bool success = false;
				auto node = std::make_unique<ExprLiteralNode>(
					SourceLocation(current_unit_name(), script_line),
					ExprLiteralNode::LiteralType::Int, int_token.value, success
				);
				if (!success) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的整数");
				}
				return node;
			};

		auto handle_float = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token float_token = current_token();
				advance();
				bool success = false;
				auto node = std::make_unique<ExprLiteralNode>(
					SourceLocation(current_unit_name(), script_line),
					ExprLiteralNode::LiteralType::Float, float_token.value, success
				);
				if (!success) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的浮点数");
				}
				return node;
			};

		auto handle_string = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token string_token = current_token();
				advance();
				bool success = false;
				auto node = std::make_unique<ExprLiteralNode>(
					SourceLocation(current_unit_name(), script_line),
					ExprLiteralNode::LiteralType::String, string_token.value, success
				);
				//额,string类型的字面量不太可能解析失败
				if (!success) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的字符串");
				}
				return node;
			};

		auto handle_char = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token char_token = current_token();
				advance();
				bool success = false;
				auto node = std::make_unique<ExprLiteralNode>(
					SourceLocation(current_unit_name(), script_line),
					ExprLiteralNode::LiteralType::Char, char_token.value, success
				);
				//额,char类型的字面量也不太可能解析失败
				if (!success) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的字符");
				}
				return node;
			};

		auto handle_bool = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token float_token = current_token();
				advance();
				bool success = false;
				auto node = std::make_unique<ExprLiteralNode>(
					SourceLocation(current_unit_name(), script_line),
					ExprLiteralNode::LiteralType::Bool, float_token.value, success
				);
				//额,bool类型的字面量也不太可能解析失败
				if (!success) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的布尔值");
				}
				return node;
			};

		switch (type) {
		case TokenType::LiteralInt:
			return handle_int();
		case TokenType::LiteralFloat:
			return handle_float();
		case TokenType::LiteralString:
			return handle_string();
		case TokenType::LiteralChar:
			return handle_char();
		case TokenType::LiteralBool:
			return handle_bool();
		default:
			break;
		}
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_grouping() {
		//start with LeftParen
		if (check(TokenType::LeftParen) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的括号表达式");
		}
		advance();

		auto expr = parent_parser_->expression_parser_->parse_expression();
		//然而事实上parse_expression中如果遇到无法识别的表达式会直接抛出错误
		if (!expr) {
			SCRIPT_PARSER_THROW_ERROR("无法解析的表达式");
		}

		if (check(TokenType::RightParen) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的右括号");
		}
		advance();
		//括号表达式需要处理后缀表达式
		return parse_postfix(std::move(expr));
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_postfix(
		std::unique_ptr<AbstractSyntaxTree> left
	) {
		//start with postfix
		//(FunctionCall,ArrayIndex,MemberAccess)
		//(LeftParen,LeftBracket,Dot)
		//else return left

		if (left == nullptr) {
			SCRIPT_PARSER_THROW_ERROR("Logical Error");
		}
		//这里不使用while是因为后缀表达式的处理交给子函数
		TokenType type = current_token().type;
		switch (type) {
		case TokenType::LeftParen:
			return parse_function_call(std::move(left));
		case TokenType::LeftBracket:
			return parse_index(std::move(left));
		case TokenType::Dot:
			return parse_member(std::move(left));
		default:
			break;
		}

		return left;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_function_call(
		std::unique_ptr<AbstractSyntaxTree> callee
	) {
		//start with LeftParen
		if (check(TokenType::LeftParen) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的函数调用");
		}
		std::size_t script_line = current_token().line;
		advance();

		//解析参数列表
		std::vector<std::unique_ptr<AbstractSyntaxTree>> args;
		if (check(TokenType::RightParen) == false) {
			do {
				auto arg = parent_parser_->expression_parser_->parse_expression();
				if (!arg) {
					SCRIPT_PARSER_THROW_ERROR("无法解析的函数参数");
				}
				args.push_back(std::move(arg));
			} while (match(TokenType::Comma));
		}
		consume(
			TokenType::RightParen, "函数调用缺少结尾的')'", __LINE__, __func__
		);//skip ')'
		//函数调用需要处理后缀表达式
		return parse_postfix(
			std::make_unique<ExprCallNode>(
				SourceLocation(current_unit_name(), script_line),
				std::move(callee), args
			)
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_index(
		std::unique_ptr<AbstractSyntaxTree> var
	) {
		//start with LeftBracket
		if (check(TokenType::LeftBracket) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的数组下标");
		}
		std::size_t script_line = current_token().line;
		advance();//skip '['

		auto index = parent_parser_->expression_parser_->parse_expression();
		//然而事实上parse_expression中如果遇到无法识别的表达式会直接抛出错误
		if (!index) {
			SCRIPT_PARSER_THROW_ERROR("无法解析的数组下标");
		}

		if (check(TokenType::RightBracket) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的右括号");
		}
		advance();
		//数组下标需要处理后缀表达式
		return parse_postfix(
			std::make_unique<ExprIndexNode>(
				SourceLocation(current_unit_name(), script_line),
				std::move(var), std::move(index)
			)
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_member(
		std::unique_ptr<AbstractSyntaxTree> var
	) {
		//start with Dot
		if (check(TokenType::Dot) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的成员访问");
		}
		std::size_t script_line = current_token().line;
		advance();//skip '.'
		Token member_token = consume(
			TokenType::Identifier, "成员访问中缺少成员名称", __LINE__, __func__
		);
		//成员访问需要处理后缀表达式
		return parse_postfix(
			std::make_unique<ExprMemberNode>(
				SourceLocation(current_unit_name(), script_line),
				std::move(var), member_token.value
			)
		);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_unary() {
		//start with Unary or Primary
		//(InternalVar,LocalVar,LogicalNot,Minus,InternalFunc,ModuleFunc)
		if (check_tag({ TokenTag::Unary,TokenTag::Primary }) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		}
		if (check_tag(TokenTag::Unary) == true) {
			Token op = current_token();
			advance();
			auto right = parse_unary();
			return parse_postfix(std::make_unique<ExprUnaryNode>(
				SourceLocation(current_unit_name(), op.line),
				op.type, std::move(right)
			));
		}
		return parse_primary();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_binary(
		std::unique_ptr<AbstractSyntaxTree> left, Precedence precedence
	) {
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_multiplicative(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_additive(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_comparison(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_equality(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_logical_and(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_logical_or(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_assign(
		std::unique_ptr<AbstractSyntaxTree> left
	)
	{
		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_initializer_list()
	{
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	Parser::ExpressionParser::Precedence Parser::ExpressionParser::token_precedence(
		TokenType type
	) const {
		static const std::unordered_map<TokenType, Precedence> precedence_container = {
			{TokenType::Assign,Precedence::Assign },
			{TokenType::PlusAssign,Precedence::Assign },
			{TokenType::MinusAssign,Precedence::Assign },
			{TokenType::MultiplyAssign,Precedence::Assign },
			{TokenType::DivideAssign,Precedence::Assign },
			{TokenType::ModuloAssign,Precedence::Assign },

			{TokenType::LogicalOr,Precedence::LogicalOr },
			{TokenType::LogicalAnd,Precedence::LogicalAnd },

			{TokenType::Equal,Precedence::Equality },
			{TokenType::NotEqual,Precedence::Equality },

			{TokenType::Greater,Precedence::Comparison },
			{TokenType::GreaterEqual,Precedence::Comparison },
			{TokenType::Less,Precedence::Comparison },
			{TokenType::LessEqual,Precedence::Comparison },

			{TokenType::Plus,Precedence::Additive },
			{TokenType::Minus,Precedence::Additive },

			{TokenType::Multiply,Precedence::Multiplicative },
			{TokenType::Divide,Precedence::Multiplicative },
			{TokenType::Modulo,Precedence::Multiplicative },

			{TokenType::LogicalNot,Precedence::Unary },

			{TokenType::LeftParen,Precedence::Call },
			{TokenType::LeftBracket,Precedence::Index },
			{TokenType::Dot,Precedence::Member }
		};
		auto iter = precedence_container.find(type);
		if (iter != precedence_container.cend()) {
			return iter->second;
		}
		return Precedence::None;
	}

	Parser::ExpressionParser::Associativity Parser::ExpressionParser::token_associativity(
		TokenType type
	) const {
		static const std::unordered_map<TokenType, Associativity> associativity_container = {
			{TokenType::Assign,Associativity::Right },
			{TokenType::PlusAssign,Associativity::Right },
			{TokenType::MinusAssign,Associativity::Right },
			{TokenType::MultiplyAssign,Associativity::Right },
			{TokenType::DivideAssign,Associativity::Right },
			{TokenType::ModuloAssign,Associativity::Right },

			{TokenType::LogicalOr,Associativity::Left },
			{TokenType::LogicalAnd,Associativity::Left },

			{TokenType::Equal,Associativity::Left },
			{TokenType::NotEqual,Associativity::Left },

			{TokenType::Greater,Associativity::Left },
			{TokenType::GreaterEqual,Associativity::Left },
			{TokenType::Less,Associativity::Left },
			{TokenType::LessEqual,Associativity::Left },

			{TokenType::Plus,Associativity::Left },
			{TokenType::Minus,Associativity::Left },

			{TokenType::Multiply,Associativity::Left },
			{TokenType::Divide,Associativity::Left },
			{TokenType::Modulo,Associativity::Left },

			{TokenType::LogicalNot,Associativity::Right },

			{TokenType::LeftParen,Associativity::Left },
			{TokenType::LeftBracket,Associativity::Left },
			{TokenType::Dot,Associativity::Left }
		};
		auto iter = associativity_container.find(type);
		if (iter != associativity_container.cend()) {
			return iter->second;
		}
		return Associativity::None;
	}

	std::pair<
		Parser::ExpressionParser::Precedence,
		Parser::ExpressionParser::Associativity
	> 	Parser::ExpressionParser::current_token_attribute() const
	{
		return std::pair<Precedence, Associativity>(
			token_precedence(current_token().type),
			token_associativity(current_token().type)
		);
	}


#undef SCRIPT_PARSER_COMPILE_WARNING
#undef SCRIPT_PARSER_COMPILE_ERROR
#undef SCRIPT_PARSER_THROW_ERROR
#undef SCRIPT_PARSER_THROW_ERROR_HIDE_TOKEN
}