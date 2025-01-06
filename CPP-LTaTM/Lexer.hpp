#pragma once

#include "DFA.hpp"

enum class TokenType
{
	Id = 0,
	Operator,
	FloatLiteral,
	IntLiteral,
	StringLiteral,
	Separator,
	Keyword
};

struct Token
{
public:
	TokenType type = TokenType::Separator;
	string value;
	uint32_t line = 0;
	uint32_t symbol = 0;

	Token(const char value_[]) : 
		value  (value_) { }

	Token(string const& value_) : 
		value  (value_) { }

	Token(TokenType type_, string const& value_, uint32_t line_, uint32_t symbol_) : 
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

inline ostream& operator << (ostream& os, Token const& token)
{
	os << token.value;
	return os;
}

struct TokenHash
{
	inline size_t operator () (Token const& token) const
	{
		return hash<int>()(int(token.type)) ^ hash<string>()(token.value);
	}
};



class Lexer
{
public:
	Lexer(filesystem::path srcPath);

	vector<Token> const& tokenize();
	
	inline static string tokenTypeToString(TokenType type)
	{
		switch (type)
        {
        case TokenType::Id: return "Идентификатор";
            break;
        case TokenType::Operator: return "Оператор";
            break;
        case TokenType::FloatLiteral: return "Литерал (float)";
            break;
        case TokenType::IntLiteral: return "Литерал (int)";
            break;
        case TokenType::StringLiteral: return "Литерал (string)";
            break;
        case TokenType::Separator: return "Разделитель";
            break;
        case TokenType::Keyword: return "Ключевое слово";
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