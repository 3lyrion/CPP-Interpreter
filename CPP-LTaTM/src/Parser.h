#pragma once

#include <Lexer.h>
#include <Shell.h>

class Parser
{
public:
	using TTree    = Tree<Shell::Token>;
	using TTreePtr = uptr<TTree>;

	Parser() = default;

	TTreePtr parse(vector<Lexer::Token> const& theTokens);

private:
	vector<Lexer::Token> const* tokens{};
	Lexer::Token         const* token{};

	TTreePtr m_tree{};

	size_t m_index{};
	size_t m_trace{};

	unordered_set<Shell::Token, Shell::Token::Hash> m_exclude =
	{
		"const", "program", "block", "statement", "typeSpec",
		"logic1", "logic2", "logic3", "term", "factor", "power", "operand",
		"literal", "id", ";"/*, "iterStmt", "selStmt", "printStmt", "exprStmt"*/
	};

private:
	void fatalError();

	void seek();

	Lexer::Token const* peek();

	void taste(Lexer::Token::Type type);

	void eat(Lexer::Token::Type type);

	void raise();

	void compare(string const& value);

private:
	void program(TTree& tree);

	TTreePtr declaration();

	void typeSpec(TTree& tree);

	TTreePtr expression();

	void logic1(TTree& tree);

	void logic2(TTree& tree);

	void logic3(TTree& tree);

	void term(TTree& tree);

	void factor(TTree& tree);

	void power(TTree& tree);

	void operand(TTree& tree);

	void literal(TTree& tree);

	void statement(TTree& tree);

	TTreePtr selStmt();

	void block(TTree& tree);

	TTreePtr iterStmt();

	TTreePtr printStmt();

	TTreePtr exprStmt();
};

