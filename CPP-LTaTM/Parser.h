#pragma once

#include "Lexer.hpp"
#include "Shell.h"

class Parser
{
public:
	using TTree    = Tree<Shell::Token>;
	using TTreePtr = uptr<TTree>;

	Parser(Lexer& theLexer);

	TTreePtr parse();

private:
	Lexer& lexer;

	vector<Lexer::Token> const* tokens = nullptr;
	Lexer::Token         const* token  = nullptr;

	TTreePtr m_tree = nullptr;

	size_t m_index = 0;
	size_t m_trace = 0;

	unordered_set<Shell::Token, Shell::Token::Hash> m_exclude =
	{
		"const", "program", "block", "statement", "typeSpec",
		"logic1", "logic2", "logic3", "term", "factor", "power", "operand",
		"literal", "id", ";"/*, "iterStmt", "selStmt", "printStmt", "exprStmt"*/
	};

private:
	void seek();

	Lexer::Token const* peek();

	void eat(Lexer::Token::Type type);

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

