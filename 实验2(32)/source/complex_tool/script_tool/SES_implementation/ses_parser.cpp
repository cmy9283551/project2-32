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

	Parser::Parser(const ParserDependence& dependence)
		:error_recoerer_(std::make_unique<ErrorRecoverer>(*this)),
		statement_parser_(std::make_unique<StatementParser>(*this)),
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
			return asts;
		}
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

	Parser::TokenTag Parser::find_tag(TokenType type) const {
		static const std::unordered_map<TokenType, TokenTag> tag_container = {
			{TokenType::Const,TokenTag::Const },
			//TypeName
			{TokenType::Int,TokenTag::TypeName },
			{TokenType::Float,TokenTag::TypeName },
			{TokenType::Char,TokenTag::TypeName },
			{TokenType::String,TokenTag::TypeName },
			{TokenType::VectorInt,TokenTag::TypeName },
			{TokenType::VectorFloat,TokenTag::TypeName },
			{TokenType::Package,TokenTag::TypeName },

			//ControlFlow
			{TokenType::If,TokenTag::ControlFlow },
			{TokenType::Else,TokenTag::ControlFlow },
			{TokenType::While,TokenTag::ControlFlow },
			{TokenType::For,TokenTag::ControlFlow },

			//Constant
			{TokenType::ConstInt,TokenTag::Constant },
			{TokenType::ConstFloat,TokenTag::Constant },
			{TokenType::ConstChar,TokenTag::Constant },
			{TokenType::ConstString,TokenTag::Constant },
			{TokenType::ConstBool,TokenTag::Constant },

			//Identifier
			{TokenType::Identifier,TokenTag::Identifier },
		};

		auto iter = tag_container.find(type);
		if (iter->second == TokenTag::Identifier) {
			auto id_tag = identify();
			if (id_tag.has_value() == true) {
				return id_tag.value();
			}
			return TokenTag::Identifier;
		}
		if (iter == tag_container.end()) {
			return TokenTag::NoTag;
		}
		return iter->second;
	}

	bool Parser::check_tag(TokenTag tag) const {
		return tag == find_tag(current_token().type);
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
		:Parser(dependence),script_config_parser_(std::make_unique<ScriptConfigParser>(*this)) {
	}

	Parser::StructTemplateContainer& ScriptParser::current_stc() {
		return current_script_config_->script_stc;
	}

	const std::string& ScriptParser::current_unit_name() const {
		return current_script_name_;
	}

	std::optional<Parser::TokenTag> ScriptParser::identify()const {
		if (current_token().type != TokenType::Identifier) {
			return std::nullopt;
		}
		const std::string& identifier = current_token().value;
		std::size_t size = variable_stack_.size();

		//保证scope_visitor和module_visitor无名称冲突
		const auto& scope_visitor = current_script_config_->scope_visitor;
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
		const auto& module_visitor = current_script_config_->module_visitor;
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

	std::optional<std::unique_ptr<AbstractSyntaxTree>> ScriptParser::parse_unit() {
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

	Parser::TokenTag Parser::ChildParser::find_tag(TokenType type)const {
		return parent_parser_->find_tag(type);
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
				SCRIPT_PARSER_THROW_ERROR("不存在的配置选项")
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
				ASSERT(false);
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号")
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号")
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号")
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
			SCRIPT_PARSER_THROW_ERROR("预期外的符号")
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
		//start with '{'
		consume(
			TokenType::LeftBrace, "脚本语法块应当包含在{...}之中,但读取到的是", __LINE__, __func__
		);
		current_variable_stack().push_back(LocalVariableTable(current_stc()));
		std::vector<std::unique_ptr<AbstractSyntaxTree>> asts;
		while (check(TokenType::LeftBrace) == false) {
			if (match(TokenType::LeftBrace) == true) {
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
			if (check_tag(TokenTag::ControlFlow) == true) {
				asts.emplace_back(std::move(parse_control_flow()));
				continue;
			}
			if (check_tag({
				TokenTag::InternalFunc,TokenTag::ModuleFunc,
				TokenTag::InternalVar,TokenTag::LocalVar
				}) == true) {
				asts.emplace_back(std::move(parse_expression()));
				continue;
			}
			if (match(TokenType::Semicolon) == true) {
				continue;
			}
			SCRIPT_PARSER_THROW_ERROR("无法识别的句首token")
		}

		return std::make_unique<StmtBlockNode>(asts);
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_block() {

		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_variable_declaration() {
		TokenTag tag = find_tag(current_token().type);
		std::unique_ptr<AbstractSyntaxTree> result;
		auto handle_type = [&, this]()
			{
				Token type_name = current_token();
				advance();//skip type name
				Token var_name = consume(
					TokenType::Identifier,
					"变量声明语句中缺少变量名称",
					__LINE__, __func__
				);
			};
		auto handle_internal_type = [&, this]()
			{
			};
		auto handle_module_type = [&, this]()
			{
			};
		auto handle_const = [&, this]()
			{
				advance();//skip 'const'
			};

		switch (tag) {
		case TokenTag::TypeName:
			handle_type();
			break;
		case TokenTag::InternalType:
			handle_internal_type();
			break;
		case TokenTag::ModuleType:
			handle_module_type();
			break;
		case TokenTag::Const:
			handle_const();
			break;
		default:
			ASSERT(false);
			break;
		}
		return result;
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_control_flow() {

		return std::unique_ptr<AbstractSyntaxTree>();
	}

	std::unique_ptr<AbstractSyntaxTree> Parser::StatementParser::parse_expression() {

		return std::unique_ptr<AbstractSyntaxTree>();
	}

}