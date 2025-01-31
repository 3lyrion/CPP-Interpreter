#pragma once

#include <Precompiled.h>

class Lexer
{
public:
	struct Token
	{
	public:
		enum class Type
		{
			Separator = 0,
			Id,
			Operator,
			FloatLiteral,
			IntLiteral,
			StringLiteral,
			Keyword
		};

		Type     type{};
		string   value;
		uint32_t line{};
		uint32_t column{};

		Token(const char value_[]) : 
			value(value_) { }

		Token(string const& value_) : 
			value(value_) { }

		Token(Type type_, string const& value_, uint32_t line_, uint32_t column_) : 
			type	(type_),
			value	(value_),
			line	(line_),
			column	(column_) { }

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

		friend ostream& operator << (ostream&, Token::Type const&);
	
		friend ostream& operator << (ostream&, Token const&);
	};

	vector<Token> const& tokenize(fs::path const& path);

	inline vector<Token> const& getTokens() const { return m_tokens; }

private:
	string		m_src;
	size_t      m_pos{};
	char        m_char{};
	size_t      m_srcLength{};
	uint32_t	m_curLine	= 1;
	uint32_t	m_curColumn	= 0;
	
	vector<Token> m_tokens;

	size_t advance(size_t n = 1);

	char peek() const;

	bool hasCharsLeft() const;

	void skipWhitespace();

	void skipComment();

	bool eatNextToken();

	void throwError(string const& msg) const;
};

inline ostream& operator << (ostream& os, Lexer::Token::Type const& type)
{
	using LType = Lexer::Token::Type;

	switch (type)
	{
	case LType::FloatLiteral:
		os << "float_literal";
	break;

	case LType::Id:
		os << "id";
	break;

	case LType::IntLiteral:
		os << "int_literal";
	break;

	case LType::Keyword:
		os << "keyword";
	break;

	case LType::Operator:
		os << "operator";
	break;

	case LType::Separator:
		os << "separator";
	break;

	case LType::StringLiteral:
		os << "string_literal";
	break;

	default:
		break;
	}

	return os;
}

inline ostream& operator << (ostream& os, Lexer::Token const& token)
{
	os << token.value;
	return os;
}