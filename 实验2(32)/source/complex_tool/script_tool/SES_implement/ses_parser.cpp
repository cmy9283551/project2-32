#include "complex_tool/script_tool/SES_implement/ses_parser.h"

SESParser::SESParser()
	:error_recoerer_(new ErrorRecoverer(this)),
	config_parser_(new ConfigParser(this)) {
}

std::vector<std::unique_ptr<SESAbstractSyntaxTree>> SESParser::parse_ses(
	const std::string& script_path,
	const SESCompileDependence& dependence
) {
	current_file_path_ = script_path;
	std::vector<std::unique_ptr<SESAbstractSyntaxTree>> asts;
	SESTokenStream token_stream(script_path);
	current_token_stream_ = &token_stream;
	current_dependence_ = &dependence;

	SESLexer lexer;
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

std::unique_ptr<SESAbstractSyntaxTree> SESParser::parse_ses_script() {
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
	return std::unique_ptr<SESAbstractSyntaxTree>();
}

SESToken SESParser::current_token() const {
	if (current_token_stream_ == nullptr) {
		ASSERT(false);
	}
	return current_token_stream_->current_token();
}

void SESParser::advance() {
	if (current_token_stream_ == nullptr) {
		ASSERT(false);
	}
	current_token_stream_->advance();
}

bool SESParser::check(TokenType type) const {
	return current_token_stream_->current_token().type == type;
}

bool SESParser::match(TokenType type) {
	if (check(type) == true) {
		advance();
		return true;
	}
	return false;
}

bool SESParser::is_at_end()const {
	return current_token_stream_->is_at_end();
}

void SESParser::panic_mode_recovery(PanicEnd end) {
	error_recoerer_->panic_mode(end);
}

SESParser::ErrorRecoverer::ErrorRecoverer(SESParser* parent_parser)
	:parent_parser_(parent_parser) {
}

void SESParser::ErrorRecoverer::panic_mode(PanicEnd end) {
	switch (end)
	{
	case SESParser::PanicEnd::RightParen:
		panic_mode_mult(end);
		break;
	case SESParser::PanicEnd::RightBracket:
		panic_mode_mult(end);
		break;
	case SESParser::PanicEnd::RightBrace:
		panic_mode_mult(end);
		break;
	case SESParser::PanicEnd::Semicolon:
		panic_mode_common(end);
		break;
	case SESParser::PanicEnd::Comma:
		panic_mode_common(end);
		break;
	default:
		ASSERT(false);
		break;
	}
}

void SESParser::ErrorRecoverer::panic_mode_common(PanicEnd end) {
	TokenType type;
	switch (end)
	{
	case SESParser::PanicEnd::Semicolon:
		type = TokenType::Semicolon;
		break;
	case SESParser::PanicEnd::Comma:
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

void SESParser::ErrorRecoverer::panic_mode_mult(PanicEnd end) {
	TokenType left, right;
	switch (end)
	{
	case SESParser::PanicEnd::RightParen:
		left = TokenType::LeftParen;
		right = TokenType::RightParen;
		break;
	case SESParser::PanicEnd::RightBracket:
		left = TokenType::LeftBracket;
		right = TokenType::RightBracket;
		break;
	case SESParser::PanicEnd::RightBrace:
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

const std::unordered_map<std::string, SESParser::ConfigParser::Keyword>
SESParser::ConfigParser::keyword_list_ = {
	{"module",SESParser::ConfigParser::Keyword::Module},
	{"parameter",SESParser::ConfigParser::Keyword::Input},
	{"return_value",SESParser::ConfigParser::Keyword::OutPut},
	{"variable_scope",SESParser::ConfigParser::Keyword::VariableScope},
	{"function_scope",SESParser::ConfigParser::Keyword::FunctionScope}
};

SESParser::ConfigParser::ConfigParser(SESParser* parent_parser)
	:parent_parser_(parent_parser) {
}

std::shared_ptr<SESScriptConfig> SESParser::ConfigParser::parse_ses_script_config() {
	std::unique_ptr<SESScriptConfig> ptr(new SESScriptConfig(
		parent_parser_->current_dependence_->default_script_config
	));
	if (parent_parser_->check(TokenType::LeftBracket) == false) {
		return ptr;
	}
	parent_parser_->advance();//skip'['
	std::vector<std::string> variable_scope, function_scope;
	while (parent_parser_->check(TokenType::RightBracket) == false) {
		if (parent_parser_->match(TokenType::Identifier) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				parent_parser_->current_file_path_, parent_parser_->current_script_name_
			) << "脚本配置列表中出现错误符号" << parent_parser_->current_token()
				<< ",而此处需要一个Identifier\n";
			parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
			return ptr;
		}
		auto iter = keyword_list_.find(parent_parser_->current_token().value);
		if (iter == keyword_list_.end()) {
			SCRIPT_PARSER_COMPILE_ERROR(
				parent_parser_->current_file_path_, parent_parser_->current_script_name_
			) << "不存在配置选项[" << parent_parser_->current_token().value << "]\n";
			parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
			return ptr;
		}
		switch (iter->second)
		{
		case SESParser::ConfigParser::Keyword::Module:
			if (parse_module_list(ptr->module_visitor) == false) {
				parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			break;
		case SESParser::ConfigParser::Keyword::Input:
			if (parse_input_parameter(ptr->input) == false) {
				parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			break;
		case SESParser::ConfigParser::Keyword::OutPut:
			if (parse_output_parameter(ptr->output) == false) {
				parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			break;
		case SESParser::ConfigParser::Keyword::VariableScope:
			if (parse_variable_scope(variable_scope) == false) {
				parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			break;
		case SESParser::ConfigParser::Keyword::FunctionScope:
			if (parse_function_scope(function_scope) == false) {
				parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
				return ptr;
			}
			break;
		default:
			ASSERT(false);
			break;
		}
	}
	parent_parser_->advance();//skip']'

	auto result = parent_parser_->current_dependence_->scope_visitor.init_sub_scope(
		variable_scope, function_scope, ptr->scope_visitor
	);
	if (result != std::nullopt) {
		SCRIPT_PARSER_COMPILE_ERROR(
			parent_parser_->current_file_path_, parent_parser_->current_script_name_
		) << "未找到以下作用域\n";
		ScopeVisitor::ScopeNotFound& value = result.value();
		std::size_t size = value.variable_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			SCRIPT_COMPILE_ERROR << "[variable scope]:" << value.variable_scope[i];
		}
		size = value.function_scope.size();
		for (std::size_t i = 0; i < size; i++) {
			SCRIPT_COMPILE_ERROR << "[function scope]:" << value.function_scope[i];
		}
	}
	return ptr;
}

bool SESParser::ConfigParser::parse_module_list(SESModuleVisitor& module_list) {

}

bool SESParser::ConfigParser::parse_variable_scope(std::vector<std::string>& variable_scope) {
}

bool SESParser::ConfigParser::parse_function_scope(std::vector<std::string>& function_scope) {
}

bool SESParser::ConfigParser::parse_input_parameter(SESScriptParameter& input) {
}

bool SESParser::ConfigParser::parse_output_parameter(SESScriptParameter& output) {
}

std::unique_ptr<SESStatementNode> SESRecursiveDescentParser::parse_ses_statement()const {

	return std::unique_ptr<SESStatementNode>();
}

std::unique_ptr<SESStatementNode> SESPrattParser::parse_ses_statement()const {

	return std::unique_ptr<SESStatementNode>();
}
