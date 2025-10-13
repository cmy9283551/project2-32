#include "complex_tool/script_tool/SES_implementation/ses_parser.h"

namespace ses {

	LocalVariableTable::LocalVariableTable(
		StructTemplateContainer& struct_template_container
	) :struct_template_container_(&struct_template_container) {
	}

	bool LocalVariableTable::push_back(
		const std::string& type_name, const std::string& var_name
	) {
		auto result = struct_template_container_->find(type_name);
		if (result == std::nullopt) {
			return false;
		}
		variable_table_.insert(type_name, result.value());
		return true;
	}

	bool LocalVariableTable::contains(const std::string& var_name) const {
		return variable_table_.contains(var_name);
	}

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

	std::optional<Parser::TokenTag> Parser::identify()const {
		if (current_token().type != TokenType::Identifier) {
			return std::nullopt;
		}
		const std::string& identifier = current_token().value;
		auto iter = variable_stack_.crbegin();
		for (; iter != variable_stack_.crend(); iter++) {
			if (iter->contains(identifier) == true) {
				return TokenTag::LocalVar;
			}
		}
		//保证scope_visitor和module_visitor无名称冲突
		const auto& scope_visitor = current_scope_visitor();
		auto sv_result = scope_visitor.identify(identifier);
		if (sv_result.has_value() == true) {
			switch (sv_result.value())
			{
			case ScopeVisitor::IdentifierType::Variable:
				return TokenTag::InternalVar;
			case ScopeVisitor::IdentifierType::TypeName:
				return TokenTag::InternalType;
			case ScopeVisitor::IdentifierType::Function:
				return TokenTag::InternalFunc;
			default:
				ASSERT(false);
				break;
			}
		}
		const auto& module_visitor = current_module_visitor();
		auto mv_result = module_visitor.identify(identifier);
		if (mv_result.has_value() == true) {
			switch (mv_result.value())
			{
			case ModuleVisitor::IdentifierType::Function:
				return TokenTag::ModuleFunc;
			case ModuleVisitor::IdentifierType::TypeName:
				return TokenTag::ModuleType;
			default:
				ASSERT(false);
				break;
			}
		}
		return std::nullopt;
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
			{TokenType::Identifier,{TokenTag::Identifier} },
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
		static const TokenTagTable internal_var = {
			TokenTag::InternalVar,TokenTag::Primary
		};
		static const TokenTagTable internal_type = {
			TokenTag::InternalType
		};
		static const TokenTagTable internal_func = {
			TokenTag::InternalFunc,TokenTag::Primary
		};
		static const TokenTagTable module_type = {
			TokenTag::ModuleType
		};
		static const TokenTagTable module_func = {
			TokenTag::ModuleFunc,TokenTag::Primary
		};
		static const TokenTagTable local_var = {
			TokenTag::LocalVar,TokenTag::Primary
		};
		static const TokenTagTable no_tag = {
			TokenTag::NoTag
		};

		auto iter = tag_container.find(type);
		if (iter->second.contains(TokenTag::Identifier)) {
			auto has_special_tag = identify();
			if (has_special_tag == std::nullopt) {
				return iter->second;
			}
			const auto& special_tag = has_special_tag.value();
			switch (special_tag) {
			case TokenTag::InternalVar:
				return internal_var;
			case TokenTag::InternalType:
				return internal_type;
			case TokenTag::InternalFunc:
				return internal_func;
			case TokenTag::ModuleType:
				return module_type;
			case TokenTag::ModuleFunc:
				return module_func;
			default:
				ASSERT(false);
				break;
			}
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

	Token Parser::consume(
		TokenType type,
		const std::string& message,
		std::size_t line,
		const std::string& func
	) {
		auto token = current_token();
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
				variable_stack_.clear();
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

	Token Parser::ChildParser::current_token()const {
		return parent_parser_->current_token();
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

	Token Parser::ChildParser::consume(
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

	std::list<LocalVariableTable>& Parser::ChildParser::current_variable_stack() {
		return parent_parser_->variable_stack_;
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
		current_variable_stack().push_back(LocalVariableTable(current_stc()));
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		while (check(TokenType::LeftBrace) == false) {
			if (check(TokenType::LeftBrace) == true) {
				asts.emplace_back(std::move(parse_block()));
				continue;
			}
			if (check_tag({
				TokenTag::TypeName,TokenTag::InternalType,
				TokenTag::ModuleType ,TokenTag::Const
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
			{ TokenTag::TypeName,TokenTag::InternalType,TokenTag::ModuleType,TokenTag::Const }
		);
		if (tag == TokenTag::NoTag || tag == TokenTag::MoreThanOneTag) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的变量声明语句");
		}

		auto handle_type = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token type_name = current_token();
				const auto& stc = current_stc();
				auto is_type = stc.find(type_name.value);
				if (is_type == std::nullopt) {
					SCRIPT_PARSER_THROW_ERROR("未找到该类型");
				}
				StructProxy type = { is_type.value() ,current_stc() };
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"变量声明语句中缺少变量名称",
					__LINE__, __func__
				);
				if (match(TokenType::Assign) == true) {
					return std::make_unique<StmtDeclarationNode>(
						SourceLocation(current_unit_name(), script_line),
						type, var_name.value,
						parent_parser_->statement_parser_->parse_expression()
					);
				}
				return std::make_unique<StmtDeclarationNode>(
					SourceLocation(current_unit_name(), script_line),
					type, var_name.value
				);
			};
		auto handle_internal_type = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token type_name = current_token();
				const auto& sv = current_scope_visitor();
				auto is_type = sv.find_type(type_name.value);
				if (is_type == std::nullopt) {
					SCRIPT_PARSER_THROW_ERROR("未找到该类型");
				}
				//将类型的所有相关类型复制到当前结构体管理器中
				const auto& type = is_type.value();
				type.copy_all_relative_type(current_stc());
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"变量声明语句中缺少变量名称",
					__LINE__, __func__
				);
				if (match(TokenType::Assign) == true) {
					return std::make_unique<StmtDeclarationNode>(
						SourceLocation(current_unit_name(), script_line),
						type, var_name.value,
						parent_parser_->statement_parser_->parse_expression()
					);
				}
				return std::make_unique<StmtDeclarationNode>(
					SourceLocation(current_unit_name(), script_line),
					type, var_name.value
				);
			};
		auto handle_module_type = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				Token type_name = current_token();
				const auto& mv = current_module_visitor();
				auto is_type = mv.find_type(type_name.value);
				if (is_type == std::nullopt) {
					SCRIPT_PARSER_THROW_ERROR("未找到该类型");
				}
				//将类型的所有相关类型复制到当前结构体管理器中
				const auto& type = is_type.value();
				type.copy_all_relative_type(current_stc());
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"变量声明语句中缺少变量名称",
					__LINE__, __func__
				);
				if (match(TokenType::Assign) == true) {
					return std::make_unique<StmtDeclarationNode>(
						SourceLocation(current_unit_name(), script_line),
						type, var_name.value,
						parent_parser_->statement_parser_->parse_expression()
					);
				}
				return std::make_unique<StmtDeclarationNode>(
					SourceLocation(current_unit_name(), script_line),
					type, var_name.value
				);
			};
		auto handle_const = [&, this]()->std::unique_ptr<AbstractSyntaxTree>
			{
				advance();//skip 'const'
				if (check_tag({
					TokenTag::TypeName,TokenTag::InternalType,
					TokenTag::ModuleType
					}) == false) {
					SCRIPT_PARSER_THROW_ERROR("const后应当跟随类型");
				}
				return parse_variable_declaration();
			};

		switch (tag) {
		case TokenTag::TypeName:
			return handle_type();
		case TokenTag::InternalType:
			return handle_internal_type();
		case TokenTag::ModuleType:
			return handle_module_type();
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
		//TO DO:处理后缀表达式(函数调用,数组下标,成员访问)
		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
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
			return std::make_unique<ExprUnaryNode>(
				SourceLocation(current_unit_name(), op.line),
				op.type, std::move(right)
			);
		}
		return parse_primary();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_primary() {
		//start with Primary
		//(Literal,InternalVar,LocalVar,InternalFunc,ModuleFunc,LeftParen)
		if (check_tag(TokenTag::Primary) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		}
		if (check(TokenType::LeftParen) == true) {
			return parse_grouping();
		}
		if (check_tag(TokenTag::Literal) == true) {
			return parse_literal();
		}
		if (check_tag(TokenTag::InternalVar) == true ||
			check_tag(TokenTag::LocalVar) == true) {
			return parse_variable();
		}
		if (check_tag(TokenTag::InternalFunc) == true ||
			check_tag(TokenTag::ModuleFunc) == true) {
			return parse_function();
		}
		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_variable() {
		//start with InternalVar or LocalVar
		if (check_tag({ TokenTag::InternalVar,TokenTag::LocalVar }) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的变量");
		}
		//保证是InternalVar,因为检查tag时已经保证了不是LocalVar
		if (check_tag(TokenTag::InternalVar) == true) {
			Token var_token = current_token();
			const auto& sv = current_scope_visitor();
			const auto& is_var = sv.find_variable(var_token.value);
			if (is_var == std::nullopt) {
				//理论上不可能发生,因为check_tag时已经保证了是InternalVar
				//如果发生,说明存在严重的逻辑错误
				SCRIPT_PARSER_THROW_ERROR("Logical Error");
			}
			advance();
			if (check(TokenType::Assign) == true) {
				return std::make_unique<ExprBinaryNode>(
					SourceLocation(current_unit_name(), var_token.line),
					std::make_unique<ExprInternalVarNode>(
						SourceLocation(current_unit_name(), var_token.line),
						is_var.value(), var_token.value
					),
					TokenType::Assign,
					std::move(parent_parser_->expression_parser_->parse_expression())
				);
			}
			TO_DO_ASSERT;
		}
		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_function() {
		//start with InternalFunc or ModuleFunc
		if (check_tag({ TokenTag::InternalFunc,TokenTag::ModuleFunc }) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的函数");
		}

		SCRIPT_PARSER_THROW_ERROR("无法识别的表达式");
		return nullptr;
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
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_postfix(
		std::unique_ptr<AbstractSyntaxTree> left
	) {
		//start with postfix
		//(FunctionCall,ArrayIndex,MemberAccess)
		//(LeftParen,LeftBracket,Dot)
		if (check_tag(TokenTag::Postfix) == false) {
			SCRIPT_PARSER_THROW_ERROR("无法识别的后缀表达式");
		}
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

		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_function_call(
		std::unique_ptr<AbstractSyntaxTree> callee
	) {
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_index(
		std::unique_ptr<AbstractSyntaxTree> var
	) {
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_member(
		std::unique_ptr<AbstractSyntaxTree> var
	) {
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::ExpressionParser::parse_binary(
		std::unique_ptr<AbstractSyntaxTree> left, Precedence precedence
	) {
		SCRIPT_PARSER_THROW_ERROR("Logical Error");
		return nullptr;
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

}