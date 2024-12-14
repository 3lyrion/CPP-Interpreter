#pragma once

#include "Lexer.hpp"

template <typename T>
concept TokenT = std::is_same<T, TokenType>::value;

class Parser
{
public:
	Parser() = default;

	void parse(queue<Token> const& tokens);

private:
	queue<Token> m_tokens;

	Token m_token;

private:
	void nextToken();

	Token& peek();

	void eat(TokenType type);

	template <TokenT... Args>
	void eat(Args... types)
	{
		for (auto type : types)
		{
			if (m_token.type == type)
			{
				nextToken();

				return;
			}
		}

		throw runtime_error("Неожиданный токен");
	}

	void seek(TokenType type);

private:
	void program();

	void declaration();

	void typeSpec();

	void expression();

	void logic1();

	void logic2();

	void logic3();

	void term();

	void factor();

	void power();

	void operand();

	void literal();

	void statement();

	void selStmt();

	void block();

	void iterStmt();

	void printStmt();

	void exprStmt();
};

