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
			Id = 0,
			Operator,
			FloatLiteral,
			IntLiteral,
			StringLiteral,
			Separator,
			Keyword
		};

		Type     type = Type::Separator;
		string   value;
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

	vector<Token> const& tokenize(fs::path const& path);

	inline vector<Token> const& getTokens() const { return m_tokens; }

private:
	string		m_src;
	size_t      m_pos{};
	char        m_char{};
	size_t      m_srcLength{};
	uint32_t	m_curLine	= 1;
	uint32_t	m_curSymbol	= 0;
	
	vector<Token> m_tokens;

	size_t advance(size_t n = 1);

	char peek() const;

	bool hasCharsLeft() const;

	void skipWhitespace();

	void skipComment();

	bool eatNextToken();

	void throwError(string const& msg) const;
};

inline ostream& operator << (ostream& os, Lexer::Token const& token)
{
	os << token.value;
	return os;
}