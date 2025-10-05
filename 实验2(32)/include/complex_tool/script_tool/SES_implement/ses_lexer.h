#pragma once

#include <vector>
#include <unordered_map>
#include <fstream>

#include "tool/script_tool/script_debug_tool.h"
#include "complex_tool/script_tool/variable_manager.h"

//TokenΪ�ʷ�������Ľ��
struct SESToken {
	enum class TokenType {
		//Keyword 
		If, Else, While, For, Return, Break, Continue, Const,
		Int, Float, Char, String, VectorInt, VectorFloat, Package,
		//Operator
		Plus, Minus, Multiply, Divide, Modulus, Assign,
		Equal, NotEqual, Greater, Less, GreaterEqual, LessEqual,
		LogicalAnd, LogicalOr, LogicalNot,
		LeftBracket, RightBracket, Dot,
		//Delimiter
		LeftParen, RightParen,
		LeftBrace, RightBrace,
		Semicolon, Comma,
		//Constant
		ConstInt, ConstFloat, ConstChar, ConstString, ConstBool,
		//Identifier
		Identifier,
		//End
		EndOfFile
	};

	static const std::string& token_type_to_string(TokenType type);
	friend std::ostream& operator<<(std::ostream& os, const SESToken& token);

	TokenType type;
	std::string value;
	std::size_t line;
};

class SESTokenStream {
	friend class SESLexer;
	friend class SESTokenIdentifier;
public:
	SESTokenStream(const std::string& file_path);

	void advance();
	void retreat();
	const SESToken& current_token()const;
	SESToken::TokenType end()const;

	friend std::ostream& operator<<(std::ostream& os, const SESTokenStream& token_stream);
private:
	static SESToken::TokenType end_of_file_;

	std::string file_path_;
	std::vector<SESToken> tokens_;
	std::size_t current_index_ = 0;
};

//�ʷ�������,���ű��ı�ת��ΪToken��
//�ڲ�ת��ͨ������ʽ���ʵ��,������˽����Դ
class SESLexer {
public:
	//��һ�������ļ�·���Ŀյ�token_stream���
	bool tokenize(SESTokenStream& token_stream)const;
private:
	struct InFileStream {
	public:
		InFileStream(const std::string& file_path_);
		void advance();

		char current_char;
		std::size_t line;
		std::string file_path;
		std::ifstream stream;
	};

	static bool is_character(char ch);
	static bool is_number(char ch);
	static bool is_legal_identifier_char(char ch);
	static bool is_legal_sysmbol(char ch);

	bool read_const_number(
		InFileStream& file_stream, std::vector<SESToken>& tokens
	)const;
	bool read_const_string(
		InFileStream& file_stream, std::vector<SESToken>& tokens
	)const;

	bool read_identifier_or_keyword(
		InFileStream& file_stream, std::vector<SESToken>& tokens
	)const;
	bool read_operator_or_delimiter(
		InFileStream& file_stream, std::vector<SESToken>& tokens
	)const;
	void skip_comment(InFileStream& file_stream)const;
	void skip_whitespace(InFileStream& file_stream)const;

	static const std::unordered_map<std::string, SESToken::TokenType> tokens_map_;
};