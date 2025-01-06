#pragma once

#include "DFA.hpp"






//
//struct TokenHash
//{
//	inline size_t operator () (Token const& token) const
//	{
//		return hash<int>()(int(token.type)) ^ hash<string>()(token.value);
//	}
//};



class Lexer
{
public:
	struct Token
	{
	public:
		enum class Type
		{
			Id = 0,
			Operator,
			FloatLiteral,
			IntLiteral,
			StringLiteral,
			Separator,
			Keyword
		};

		Type type = Type::Separator;
		string value;
		uint32_t line = 0;
		uint32_t symbol = 0;

		Token(const char value_[]) : 
			value  (value_) { }

		Token(string const& value_) : 
			value  (value_) { }

		Token(Type type_, string const& value_, uint32_t line_, uint32_t symbol_) : 
			type   (type_),
			value  (value_),
			line   (line_),
			symbol (symbol_) { }

		inline bool operator == (Token const& token) const
		{
			if (type == token.type && value == token.value)
				return true;

			return false;
		}

		inline bool operator != (Token const& token) const
		{
			if (type != token.type || value != token.value)
				return true;

			return false;
		}
	
		friend ostream& operator << (ostream&, Token const&);
	};

	Lexer(filesystem::path srcPath);

	vector<Token> const& tokenize();
	
	inline static string tokenTypeToString(Token::Type type)
	{
		switch (type)
        {
        case Token::Type::Id: return "Идентификатор";
            break;
        case Token::Type::Operator: return "Оператор";
            break;
        case Token::Type::FloatLiteral: return "Литерал (float)";
            break;
        case Token::Type::IntLiteral: return "Литерал (int)";
            break;
        case Token::Type::StringLiteral: return "Литерал (string)";
            break;
        case Token::Type::Separator: return "Разделитель";
            break;
        case Token::Type::Keyword: return "Ключевое слово";
            break;
        default:
            break;
		}
			
	}

private:
	filesystem::path m_srcPath;

	string   m_src;
	size_t   m_pos              = 0;
	size_t   m_srcLength        = 0;
	uint32_t m_currentLine      = 1;
	uint32_t m_currentChar      = 0;
	uint32_t m_commentStartLine = 0;

	vector<Token> m_tokens;

	unordered_set<string> m_keywords;
	unordered_set<string> m_operators;
	unordered_set<string> m_compounds;

	DFA m_regex_id;
	DFA m_regex_int;
	DFA m_regex_float;
	DFA m_regex_string;
	DFA m_regex_multComment;

	void initDFA();

	bool isWhitespace(char c);

	bool isSeparator(char c);

	bool isOperator(char c);

	bool isNumber(char c);
};

inline ostream& operator << (ostream& os, Lexer::Token const& token)
{
	os << token.value;
	return os;
}