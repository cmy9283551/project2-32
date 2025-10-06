#pragma once

#include <vector>
#include <unordered_map>
#include <fstream>

#include "tool/script_tool/script_debug_tool.h"
#include "complex_tool/script_tool/variable_manager.h"

namespace ses {
	//TokenΪ�ʷ�������Ľ��
	struct Token {
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
		friend std::ostream& operator<<(std::ostream& os, const Token& token);

		TokenType type;
		std::string value;
		std::size_t line;
	};

	class TokenStream {
		friend class Lexer;
		friend class SESTokenIdentifier;
	public:
		TokenStream(const std::string& file_path);

		void advance();
		void retreat();
		const Token& current_token()const;
		const Token& last_token()const;
		Token::TokenType end()const;
		bool is_at_end()const;

		friend std::ostream& operator<<(std::ostream& os, const TokenStream& token_stream);
	private:
		static Token::TokenType end_of_file_;

		std::string file_path_;
		std::vector<Token> tokens_;
		std::size_t current_index_ = 0;
	};

	//�ʷ�������,���ű��ı�ת��ΪToken��
	//�ڲ�ת��ͨ������ʽ���ʵ��,������˽����Դ
	class Lexer {
	public:
		using TokenType = Token::TokenType;
		//��һ�������ļ�·���Ŀյ�token_stream���
		bool tokenize(TokenStream& token_stream)const;
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
			InFileStream& file_stream, std::vector<Token>& tokens
		)const;
		bool read_const_string(
			InFileStream& file_stream, std::vector<Token>& tokens
		)const;

		bool read_identifier_or_keyword(
			InFileStream& file_stream, std::vector<Token>& tokens
		)const;
		bool read_operator_or_delimiter(
			InFileStream& file_stream, std::vector<Token>& tokens
		)const;
		void skip_comment(InFileStream& file_stream)const;
		void skip_whitespace(InFileStream& file_stream)const;

		static const std::unordered_map<std::string, Token::TokenType> tokens_map_;
	};
}