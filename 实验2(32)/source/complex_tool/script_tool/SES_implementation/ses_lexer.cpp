#include "complex_tool/script_tool/SES_implementation/ses_lexer.h"

#include "tool/script_tool/script_debug_tool.h"

namespace ses {
	const std::string& Token::token_type_to_string(TokenType type) {
		static const std::unordered_map<TokenType, std::string> token_to_string_map = {
			//Keyword 
			{Token::TokenType::If,"If"},
			{Token::TokenType::Else,"Else"},
			{Token::TokenType::While,"While"},
			{Token::TokenType::For,"For"},
			{Token::TokenType::Return,"Return"},
			{Token::TokenType::Break,"Break"},
			{Token::TokenType::Continue,"Continue"},
			{Token::TokenType::Const,"Const"},
			//TypeName
			{Token::TokenType::Int,"Int"},
			{Token::TokenType::Float,"Float"},
			{Token::TokenType::Char,"Char"},
			{Token::TokenType::String,"String"},
			{Token::TokenType::VectorInt,"VectorInt"},
			{Token::TokenType::VectorFloat,"VectorFloat"},
			{Token::TokenType::Package,"Package"},
			//Operator
			{Token::TokenType::Plus,"Plus"},
			{Token::TokenType::Minus,"Minus"},
			{Token::TokenType::Multiply,"Multiply"},
			{Token::TokenType::Divide,"Divide"},
			{Token::TokenType::Modulus,"Modulus"},
			{Token::TokenType::Assign,"Assign"},
			{Token::TokenType::Equal,"Equal"},
			{Token::TokenType::NotEqual,"NotEqual"},
			{Token::TokenType::Greater,"Greater"},
			{Token::TokenType::Less,"Less"},
			{Token::TokenType::GreaterEqual,"GreaterEqual"},
			{Token::TokenType::LessEqual,"LessEqual"},
			{Token::TokenType::LogicalAnd,"LogicalAnd"},
			{Token::TokenType::LogicalOr,"LogicalOr"},
			{Token::TokenType::LogicalNot,"LogicalNot"},
			{Token::TokenType::LeftBracket,"LeftBracket"},
			{Token::TokenType::RightBracket,"RightBracket"},
			{Token::TokenType::Dot,"Dot"},
			//Delimiter
			{Token::TokenType::LeftParen,"LeftParen"},
			{Token::TokenType::RightParen,"RightParen"},
			{Token::TokenType::LeftBrace,"LeftBrace"},
			{Token::TokenType::RightBrace,"RightBrace"},
			{Token::TokenType::Semicolon,"Semicolon"},
			{Token::TokenType::Comma,"Comma"},
			//Constant
			{Token::TokenType::ConstInt,"ConstInt"},
			{Token::TokenType::ConstFloat,"ConstFloat"},
			{Token::TokenType::ConstChar,"ConstChar"},
			{Token::TokenType::ConstString,"ConstString"},
			{Token::TokenType::ConstBool,"ConstBool"},
			//Identifier
			{Token::TokenType::Identifier,"Identifier"},
			//EndOfFile
			{Token::TokenType::EndOfFile,"EndOfFile"}
		};
		auto iter = token_to_string_map.find(type);
		if (iter == token_to_string_map.cend()) {
			SCRIPT_CERR << "Error: Unknown token type." << std::endl;
			ASSERT(false);
		}
		return iter->second;
	}

	std::ostream& operator<<(std::ostream& os, const Token& token) {
		os << "<" << Token::token_type_to_string(token.type) << ","
			<< token.value << "," << token.line << ">";
		return os;
	}

	Token::TokenType TokenStream::end_of_file_ = Token::TokenType::EndOfFile;

	std::ostream& operator<<(std::ostream& os, const TokenStream& token_stream) {
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

	TokenStream::TokenStream(const std::string& file_path)
		:file_path_(file_path), current_index_(0) {
	}

	void TokenStream::advance() {
		if (current_index_ < tokens_.size() - 1) {
			current_index_++;
		}
	}

	void TokenStream::retreat() {
		if (current_index_ > 0) {
			current_index_--;
		}
	}

	const Token& TokenStream::current_token() const {
		if (tokens_.empty()) {
			SCRIPT_CERR << "Error: No tokens available." << std::endl;
			ASSERT(false);
		}
		return tokens_[current_index_];
	}

	const Token& TokenStream::last_token() const {
		if (current_index_ == 0) {
			return current_token();
		}
		return tokens_[current_index_ - 1];
	}

	Token::TokenType TokenStream::end() const {
		return end_of_file_;
	}

	bool TokenStream::is_at_end() const {
		return current_index_ == tokens_.size() - 1;
	}

	const std::unordered_map<std::string, Token::TokenType> Lexer::tokens_map_ = {
		//Keyword 
		{"if",Token::TokenType::If},
		{"else",Token::TokenType::Else},
		{"while",Token::TokenType::While},
		{"for",Token::TokenType::For},
		{"return",Token::TokenType::Return},
		{"break",Token::TokenType::Break},
		{"continue",Token::TokenType::Continue},
		{"const",Token::TokenType::Const},

		{"int",Token::TokenType::Int},
		{"float",Token::TokenType::Float},
		{"char",Token::TokenType::Char},
		{"string",Token::TokenType::String},
		{"vector_int",Token::TokenType::VectorInt},
		{"vector_float",Token::TokenType::VectorFloat},
		{"package",Token::TokenType::Package},
		//Operator
		{"+",Token::TokenType::Plus},
		{"-",Token::TokenType::Minus},
		{"*",Token::TokenType::Multiply},
		{"/",Token::TokenType::Divide},
		{"%",Token::TokenType::Modulus},
		{"=",Token::TokenType::Assign},
		{"==",Token::TokenType::Equal},
		{"!=",Token::TokenType::NotEqual},
		{">",Token::TokenType::Greater},
		{"<",Token::TokenType::Less},
		{">=",Token::TokenType::GreaterEqual},
		{"<=",Token::TokenType::LessEqual},
		{"&&",Token::TokenType::LogicalAnd},
		{"||",Token::TokenType::LogicalOr},
		{"!",Token::TokenType::LogicalNot},
		{"[",Token::TokenType::LeftBracket},
		{"]",Token::TokenType::RightBracket},
		{".",Token::TokenType::Dot},
		//Delimiter
		{"(",Token::TokenType::LeftParen},
		{")",Token::TokenType::RightParen},
		{"{",Token::TokenType::LeftBrace},
		{"}",Token::TokenType::RightBrace},
		{";",Token::TokenType::Semicolon},
		{",",Token::TokenType::Comma},
		//Constant
		{"true",Token::TokenType::ConstBool},
		{"false",Token::TokenType::ConstBool},
		{"null",Token::TokenType::ConstInt},
	};

	Lexer::InFileStream::InFileStream(const std::string& file_path_)
		:current_char(0), line(1), file_path(file_path_), stream(file_path_) {
	}

	void Lexer::InFileStream::advance() {
		stream.get(current_char);
		if (stream.eof() == true) {
			current_char = EOF;
			return;
		}
		if (current_char == '\n') {
			line++;
		}
	}

	bool Lexer::tokenize(TokenStream& token_stream) const {
		return tokenize(token_stream.file_path_, token_stream.tokens_);
	}

	bool Lexer::tokenize(std::unique_ptr<TokenStream>& token_stream) const {
		return tokenize(token_stream->file_path_, token_stream->tokens_);
	}

	bool Lexer::tokenize(
		const std::string& script_path, std::vector<Token>& tokens
	)const {
		InFileStream file_stream(script_path);
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

			SCRIPT_LEXER_COMPILE_ERROR(script_path)
				<< "词法解析时遇到无法识别的字符["
				<< file_stream.current_char << "]\n";
			return false;
		}
		tokens.emplace_back(TokenType::EndOfFile, std::string(), file_stream.line);
		return true;
	}

	bool Lexer::is_character(char ch) {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
	}

	bool Lexer::is_number(char ch) {
		return ch >= '0' && ch <= '9';
	}

	bool Lexer::is_legal_identifier_char(char ch) {
		return is_character(ch) || ch == '_' || is_number(ch);
	}

	bool Lexer::is_legal_sysmbol(char ch) {
		return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
			ch == '=' || ch == '!' || ch == '>' || ch == '<' || ch == '&' || ch == '|' ||
			ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' ||
			ch == ';' || ch == ',' || ch == '.';
	}

	bool Lexer::read_const_number(
		InFileStream& file_stream, std::vector<Token>& tokens
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
			tokens.emplace_back(TokenType::ConstFloat, number_str, line);
		}
		else {
			tokens.emplace_back(TokenType::ConstInt, number_str, line);
		}
		return true;
	}

	bool Lexer::read_const_string(
		InFileStream& file_stream, std::vector<Token>& tokens
	) const {
		std::string string_value;
		std::size_t line = file_stream.line;
		file_stream.advance();  // Skip opening quote
		while (file_stream.current_char != '\"' && file_stream.current_char != EOF) {
			string_value += file_stream.current_char;
			file_stream.advance();
		}
		if (file_stream.current_char == '\"') {
			tokens.emplace_back(TokenType::ConstString, string_value, line);
			file_stream.advance();  // Skip closing quote
			return true;
		}
		SCRIPT_LEXER_COMPILE_ERROR(file_stream.file_path)
			<< "字符串常量未闭合\n";
		return false;
	}

	bool Lexer::read_identifier_or_keyword(
		InFileStream& file_stream, std::vector<Token>& tokens
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
			tokens.emplace_back(TokenType::Identifier, identifier, line);
		}
		return true;
	}

	bool Lexer::read_operator_or_delimiter(
		InFileStream& file_stream, std::vector<Token>& tokens
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

	void Lexer::skip_comment(InFileStream& file_stream) const {
		while (file_stream.current_char != '\n' && file_stream.current_char != EOF) {
			file_stream.advance();
		}
	}

	void Lexer::skip_whitespace(InFileStream& file_stream) const {
		while (file_stream.current_char == ' ' || file_stream.current_char == '\t' ||
			file_stream.current_char == '\n' || file_stream.current_char == '\r') {
			file_stream.advance();
		}
	}
}