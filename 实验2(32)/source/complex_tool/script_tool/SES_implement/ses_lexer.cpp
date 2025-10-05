#include "complex_tool/script_tool/SES_implement/ses_lexer.h"

#include "tool/script_tool/script_debug_tool.h"

const std::string& SESToken::token_type_to_string(TokenType type) {
	static const std::unordered_map<TokenType, std::string> token_to_string_map = {
		//Keyword 
		{SESToken::TokenType::If,"If"},
		{SESToken::TokenType::Else,"Else"},
		{SESToken::TokenType::While,"While"},
		{SESToken::TokenType::For,"For"},
		{SESToken::TokenType::Return,"Return"},
		{SESToken::TokenType::Break,"Break"},
		{SESToken::TokenType::Continue,"Continue"},
		{SESToken::TokenType::Const,"Const"},
		{SESToken::TokenType::Int,"Int"},
		{SESToken::TokenType::Float,"Float"},
		{SESToken::TokenType::Char,"Char"},
		{SESToken::TokenType::String,"String"},
		{SESToken::TokenType::VectorInt,"VectorInt"},
		{SESToken::TokenType::VectorFloat,"VectorFloat"},
		{SESToken::TokenType::Package,"Package"},
		//Operator
		{SESToken::TokenType::Plus,"Plus"},
		{SESToken::TokenType::Minus,"Minus"},
		{SESToken::TokenType::Multiply,"Multiply"},
		{SESToken::TokenType::Divide,"Divide"},
		{SESToken::TokenType::Modulus,"Modulus"},
		{SESToken::TokenType::Assign,"Assign"},
		{SESToken::TokenType::Equal,"Equal"},
		{SESToken::TokenType::NotEqual,"NotEqual"},
		{SESToken::TokenType::Greater,"Greater"},
		{SESToken::TokenType::Less,"Less"},
		{SESToken::TokenType::GreaterEqual,"GreaterEqual"},
		{SESToken::TokenType::LessEqual,"LessEqual"},
		{SESToken::TokenType::LogicalAnd,"LogicalAnd"},
		{SESToken::TokenType::LogicalOr,"LogicalOr"},
		{SESToken::TokenType::LogicalNot,"LogicalNot"},
		{SESToken::TokenType::LeftBracket,"LeftBracket"},
		{SESToken::TokenType::RightBracket,"RightBracket"},
		{SESToken::TokenType::Dot,"Dot"},
		//Delimiter
		{SESToken::TokenType::LeftParen,"LeftParen"},
		{SESToken::TokenType::RightParen,"RightParen"},
		{SESToken::TokenType::LeftBrace,"LeftBrace"},
		{SESToken::TokenType::RightBrace,"RightBrace"},
		{SESToken::TokenType::Semicolon,"Semicolon"},
		{SESToken::TokenType::Comma,"Comma"},
		//Constant
		{SESToken::TokenType::ConstInt,"ConstInt"},
		{SESToken::TokenType::ConstFloat,"ConstFloat"},
		{SESToken::TokenType::ConstChar,"ConstChar"},
		{SESToken::TokenType::ConstString,"ConstString"},
		{SESToken::TokenType::ConstBool,"ConstBool"},
		//Identifier
		{SESToken::TokenType::Identifier,"Identifier"}
	};
	auto iter = token_to_string_map.find(type);
	if (iter == token_to_string_map.cend()) {
		SCRIPT_CERR << "Error: Unknown token type." << std::endl;
		ASSERT(false);
	}
	return iter->second;
}

std::ostream& operator<<(std::ostream& os, const SESToken& token) {
	os << "<" << SESToken::token_type_to_string(token.type) << ","
		<< token.value << "," << token.line << ">";
	return os;
}

std::ostream& operator<<(std::ostream& os, const SESTokenStream& token_stream) {
	std::size_t line = 0;
	for (std::size_t i = 0; i < token_stream.tokens_.size(); i++) {
		if (line != token_stream.tokens_[i].line) {
			line = token_stream.tokens_[i].line;
			if (i != 0) {
				os << "\n";
			}
			os << "Line " << line << ": ";
		}
		os << token_stream.tokens_[i] << " ";
	}
	return os;
}

SESTokenStream::SESTokenStream(const std::string& file_path)
	:file_path_(file_path) {
}

void SESTokenStream::advance() {
	if (current_index_ < tokens_.size() - 1) {
		current_index_++;
	}
}

void SESTokenStream::retreat() {
	if (current_index_ > 0) {
		current_index_--;
	}
}

const SESToken& SESTokenStream::current_token() const {
	if (tokens_.empty()) {
		SCRIPT_CERR << "Error: No tokens available." << std::endl;
		ASSERT(false);
	}
	return tokens_[current_index_];
}

bool SESTokenStream::is_end() const {
	return current_index_ == tokens_.size() - 1;
}

const std::unordered_map<std::string, SESToken::TokenType> SESLexer::tokens_map_ = {
	//Keyword 
	{"if",SESToken::TokenType::If},
	{"else",SESToken::TokenType::Else},
	{"while",SESToken::TokenType::While},
	{"for",SESToken::TokenType::For},
	{"return",SESToken::TokenType::Return},
	{"break",SESToken::TokenType::Break},
	{"continue",SESToken::TokenType::Continue},
	{"const",SESToken::TokenType::Const},

	{"int",SESToken::TokenType::Int},
	{"float",SESToken::TokenType::Float},
	{"char",SESToken::TokenType::Char},
	{"string",SESToken::TokenType::String},
	{"vector_int",SESToken::TokenType::VectorInt},
	{"vector_float",SESToken::TokenType::VectorFloat},
	{"package",SESToken::TokenType::Package},
	//Operator
	{"+",SESToken::TokenType::Plus},
	{"-",SESToken::TokenType::Minus},
	{"*",SESToken::TokenType::Multiply},
	{"/",SESToken::TokenType::Divide},
	{"%",SESToken::TokenType::Modulus},
	{"=",SESToken::TokenType::Assign},
	{"==",SESToken::TokenType::Equal},
	{"!=",SESToken::TokenType::NotEqual},
	{">",SESToken::TokenType::Greater},
	{"<",SESToken::TokenType::Less},
	{">=",SESToken::TokenType::GreaterEqual},
	{"<=",SESToken::TokenType::LessEqual},
	{"&&",SESToken::TokenType::LogicalAnd},
	{"||",SESToken::TokenType::LogicalOr},
	{"!",SESToken::TokenType::LogicalNot},
	{"[",SESToken::TokenType::LeftBracket},
	{"]",SESToken::TokenType::RightBracket},
	{".",SESToken::TokenType::Dot},
	//Delimiter
	{"(",SESToken::TokenType::LeftParen},
	{")",SESToken::TokenType::RightParen},
	{"{",SESToken::TokenType::LeftBrace},
	{"}",SESToken::TokenType::RightBrace},
	{";",SESToken::TokenType::Semicolon},
	{",",SESToken::TokenType::Comma},
	//Constant
	{"true",SESToken::TokenType::ConstBool},
	{"false",SESToken::TokenType::ConstBool},
	{"null",SESToken::TokenType::ConstInt},
};

SESLexer::InFileStream::InFileStream(const std::string& file_path_)
	:current_char(0), line(1), file_path(file_path_), stream(file_path_) {
}

void SESLexer::InFileStream::advance() {
	stream.get(current_char);
	if (stream.eof() == true) {
		current_char = EOF;
		return;
	}
	if (current_char == '\n') {
		line++;
	}
}

bool SESLexer::tokenize(
	SESTokenStream& token_stream
) const {
	InFileStream file_stream(token_stream.file_path_);
	std::vector<SESToken>& tokens = token_stream.tokens_;
	if (tokens.size() != 0 || token_stream.current_index_ != 0) {
		SCRIPT_CERR <<
			"不允许tokenize非空的token stream" << std::endl;
		ASSERT(false);
	}
	file_stream.advance();
	while (file_stream.current_char != EOF) {

		if (file_stream.current_char == '/') {
			file_stream.advance();
			if (file_stream.current_char == '/') {
				skip_comment(file_stream);
				continue;
			}
			else {
				//可能是除号
				if (read_operator_or_delimiter(file_stream, tokens) == false) {
					return false;
				}
				continue;
			}
		}

		if (file_stream.current_char == ' ' ||
			file_stream.current_char == '\t' ||
			file_stream.current_char == '\n' ||
			file_stream.current_char == '\r'
			) {
			skip_whitespace(file_stream);
			continue;
		}

		if (is_number(file_stream.current_char)) {
			if (read_const_number(file_stream, tokens) == false) {
				return false;
			}
			continue;
		}

		if (file_stream.current_char == '\"') {
			if (read_const_string(file_stream, tokens) == false) {
				return false;
			}
			continue;
		}

		if (is_character(file_stream.current_char) || file_stream.current_char == '_') {
			if (read_identifier_or_keyword(file_stream, tokens) == false) {
				return false;
			}
			continue;
		}

		if (is_legal_sysmbol(file_stream.current_char)) {
			if (read_operator_or_delimiter(file_stream, tokens) == false) {
				return false;
			}
			continue;
		}

		SCRIPT_LEXER_COMPILE_ERROR(token_stream.file_path_)
			<< "词法解析时遇到无法识别的字符["
			<< file_stream.current_char << "]\n";
		return false;
	}
	return true;
}

bool SESLexer::is_character(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool SESLexer::is_number(char ch) {
	return ch >= '0' && ch <= '9';
}

bool SESLexer::is_legal_identifier_char(char ch) {
	return is_character(ch) || ch == '_' || is_number(ch);
}

bool SESLexer::is_legal_sysmbol(char ch) {
	return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
		ch == '=' || ch == '!' || ch == '>' || ch == '<' || ch == '&' || ch == '|' ||
		ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' ||
		ch == ';' || ch == ',' || ch == '.';
}

bool SESLexer::read_const_number(
	InFileStream& file_stream, std::vector<SESToken>& tokens
) const {
	bool has_decimal_point = false;
	std::string number_str;
	std::size_t line = file_stream.line;
	while (is_number(file_stream.current_char) || file_stream.current_char == '.') {
		if (file_stream.current_char == '.') {
			if (has_decimal_point) {
				SCRIPT_LEXER_COMPILE_ERROR(file_stream.file_path)
					<< "数字常量中出现多个小数点\n";
				return false;
			}
			has_decimal_point = true;
		}
		number_str += file_stream.current_char;
		file_stream.advance();
	}
	if (has_decimal_point) {
		tokens.emplace_back(SESToken::TokenType::ConstFloat, number_str, line);
	}
	else {
		tokens.emplace_back(SESToken::TokenType::ConstInt, number_str, line);
	}
	return true;
}

bool SESLexer::read_const_string(
	InFileStream& file_stream, std::vector<SESToken>& tokens
) const {
	std::string string_value;
	std::size_t line = file_stream.line;
	file_stream.advance();  // Skip opening quote
	while (file_stream.current_char != '\"' && file_stream.current_char != EOF) {
		string_value += file_stream.current_char;
		file_stream.advance();
	}
	if (file_stream.current_char == '\"') {
		tokens.emplace_back(SESToken::TokenType::ConstString, string_value, line);
		file_stream.advance();  // Skip closing quote
		return true;
	}
	SCRIPT_LEXER_COMPILE_ERROR(file_stream.file_path)
		<< "字符串常量未闭合\n";
	return false;
}

bool SESLexer::read_identifier_or_keyword(
	InFileStream& file_stream, std::vector<SESToken>& tokens
) const {
	std::string identifier;
	std::size_t line = file_stream.line;
	while (is_legal_identifier_char(file_stream.current_char)) {
		identifier += file_stream.current_char;
		file_stream.advance();
	}
	auto it = tokens_map_.find(identifier);
	if (it != tokens_map_.cend()) {
		tokens.emplace_back(it->second, identifier, line);
	}
	else {
		tokens.emplace_back(SESToken::TokenType::Identifier, identifier, line);
	}
	return true;
}

bool SESLexer::read_operator_or_delimiter(
	InFileStream& file_stream, std::vector<SESToken>& tokens
) const {
	std::string op;
	std::size_t line = file_stream.line;
	op += file_stream.current_char;
	file_stream.advance();
	// Check for two-character operators
	if ((op == "=" && file_stream.current_char == '=') ||
		(op == "!" && file_stream.current_char == '=') ||
		(op == ">" && file_stream.current_char == '=') ||
		(op == "<" && file_stream.current_char == '=') ||
		(op == "&" && file_stream.current_char == '&') ||
		(op == "|" && file_stream.current_char == '|')) {
		op += file_stream.current_char;
		file_stream.advance();
	}
	auto it = tokens_map_.find(op);
	if (it != tokens_map_.cend()) {
		tokens.emplace_back(it->second, op, line);
		return true;
	}
	SCRIPT_LEXER_COMPILE_ERROR(file_stream.file_path)
		<< "无法识别的操作符或分隔符[" << op << "]\n";
	return false;
}

void SESLexer::skip_comment(InFileStream& file_stream) const {
	while (file_stream.current_char != '\n' && file_stream.current_char != EOF) {
		file_stream.advance();
	}
}

void SESLexer::skip_whitespace(InFileStream& file_stream) const {
	while (file_stream.current_char == ' ' || file_stream.current_char == '\t' ||
		file_stream.current_char == '\n' || file_stream.current_char == '\r') {
		file_stream.advance();
	}
}