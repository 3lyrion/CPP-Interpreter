#pragma once

#include "DFA.hpp"
	
enum class TokenType
{
	Id = 0,
	Type,
	Operator,
	BoolLiteral,
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

	Token(TokenType _type, string _value)
	: type(_type)
	, value(_value)
	{ }
};

class Lexer
{
public:
	Lexer(filesystem::path srcPath);

	vector<Token> tokenize();
	
	inline static string tokenTypeToString(TokenType type)
	{
		switch (type)
        {
        case TokenType::Id: return "Идентификатор";
            break;
        case TokenType::Type: return "Тип";
            break;
        case TokenType::Operator: return "Оператор";
            break;
        case TokenType::BoolLiteral: return "Литерал (bool)";
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