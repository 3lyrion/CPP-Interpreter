#pragma once

#include "Lexer.hpp"
#include "Tree.h"

class Parser
{
public:
	Parser(Lexer& theLexer);

	void parse();

private:
	using STree    = Tree<string>;
	using STreePtr = uptr<STree>;

	Lexer& lexer;

	vector<Token> const* tokens = nullptr;
	Token         const* token  = nullptr;

	STreePtr m_tree = nullptr;

	size_t m_index = 0;

//	list<STree> m_tempTrees;

private:
	void seek(size_t offset = 1);

	Token const& peek();

	void eat(TokenType type);

private:
	STreePtr program();

	STreePtr declaration();

	STreePtr typeSpec();

	STreePtr expression();

	STreePtr logic1();

	STreePtr logic2();

	STreePtr logic3();

	STreePtr term();

	STreePtr factor();

	STreePtr power();

	STreePtr operand();

	STreePtr literal();

	STreePtr statement();

	STreePtr selStmt();

	STreePtr block();

	STreePtr iterStmt();

	STreePtr printStmt();

	STreePtr exprStmt();
};

