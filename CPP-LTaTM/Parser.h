#pragma once

#include "Lexer.hpp"
#include "Tree.h"

class Parser
{
public:
	Parser(Lexer& theLexer);

	Tree<Token> const& parse();

private:
	using TTree    = Tree<Token>;
	using TTreePtr = uptr<TTree>;

	Lexer& lexer;

	vector<Token> const* tokens = nullptr;
	Token         const* token  = nullptr;

	TTreePtr m_tree = nullptr;

	size_t m_index = 0;
	size_t m_trace = 0;

	unordered_set<Token, TokenHash> m_exclude =
	{
		"const", "program", "block", "statement", "typeSpec",
		"logic1", "logic2", "logic3", "term", "factor", "power", "operand",
		"literal", "iterStmt", "selStmt", "printStmt", "exprStmt"
	};

private:
	void seek();

	Token const& peek();

	void eat(TokenType type);

	void raise();

private:
	TTreePtr program();

	TTreePtr declaration();

	TTreePtr typeSpec();

	TTreePtr expression();

	TTreePtr logic1();

	TTreePtr logic2();

	TTreePtr logic3();

	TTreePtr term();

	TTreePtr factor();

	TTreePtr power();

	TTreePtr operand();

	TTreePtr literal();

	TTreePtr statement();

	TTreePtr selStmt();

	TTreePtr block();

	TTreePtr iterStmt();

	TTreePtr printStmt();

	TTreePtr exprStmt();
};

