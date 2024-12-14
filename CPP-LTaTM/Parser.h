#pragma once

#include "Lexer.hpp"

template <typename T>
concept TokenT = std::is_same<T, TokenType>::value;

class Parser
{
public:
	Parser(Lexer& theLexer);

	void parse();

private:
	Lexer& lexer;

	vector<Token> const* tokens = nullptr;
	Token         const* token  = nullptr;

	size_t m_index = 0;

private:
	void seek(size_t offset = 1);

	Token const& peek();

	void eat(TokenType type);

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

