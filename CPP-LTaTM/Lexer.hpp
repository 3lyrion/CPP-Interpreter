#pragma once

#include "DFA.hpp"

#include <queue>
	
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
	TokenType type;
	string value;

	Token() : 
		type(TokenType(0)) { }

	Token(TokenType _type, string _value)
	: type(_type)
	, value(_value)
	{ }
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

	string m_src;
	size_t m_pos              = 0;
	size_t m_srcLength        = 0;
	size_t m_currentLine      = 1;
	size_t m_currentChar      = 0;
	size_t m_commentStartLine = 0;

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