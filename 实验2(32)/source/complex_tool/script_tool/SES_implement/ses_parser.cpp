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
	current_script_config_ = config_parser_->parse_ses_script_config().get();
	if (current_script_config_ == nullptr) {
		SCRIPT_PARSER_COMPILE_ERROR(current_file_path_, current_script_name_)
			<< "脚本配置解析失败,尝试使用默认配置\n";
		current_script_config_ = &current_dependence_->default_script_config;
	}
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
	while (parent_parser_->match(type) == false) {}
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
	while (count != 0) {
		if (parent_parser_->check(left)) {
			count++;
		}
		if (parent_parser_->check(right)) {
			count--;
		}
		parent_parser_->advance();
	}
}

SESParser::ConfigParser::ConfigParser(SESParser* parent_parser)
	:parent_parser_(parent_parser) {
}

std::unique_ptr<SESScriptConfig> SESParser::ConfigParser::parse_ses_script_config() const {
	std::unique_ptr<SESScriptConfig> ptr(new SESScriptConfig);
	while (parent_parser_->check(TokenType::LeftBracket) == false) {
		if (parent_parser_->match(TokenType::Identifier) == false) {
			SCRIPT_PARSER_COMPILE_ERROR(
				parent_parser_->current_file_path_, parent_parser_->current_script_name_
			) << "脚本配置列表中出现错误符号" << parent_parser_->current_token()
				<< ",而此处需要一个Identifier\n";
			parent_parser_->panic_mode_recovery(PanicEnd::RightBracket);
		}
	}
	return ptr;
}

void SESParser::ConfigParser::parse_module_list(SESModuleVisitor& module_list) {
}

void SESParser::ConfigParser::parse_variable_scope(std::vector<std::string>& variable_scope) {
}

void SESParser::ConfigParser::parse_function_scope(std::vector<std::string>& function_scope) {
}

void SESParser::ConfigParser::parse_input_parameter(SESScriptParameter& input) {
}

void SESParser::ConfigParser::parse_output_parameter(SESScriptParameter& output) {
}

std::unique_ptr<SESStatementNode> SESRecursiveDescentParser::parse_ses_statement()const {

	return std::unique_ptr<SESStatementNode>();
}

std::unique_ptr<SESStatementNode> SESPrattParser::parse_ses_statement()const {

	return std::unique_ptr<SESStatementNode>();
}
