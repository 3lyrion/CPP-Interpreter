#include <Parser.h>

using LType = Lexer::Token::Type;
using TType = Shell::Token::Type;
using VType = Shell::Token::Value::Type;

Parser::TTreePtr Parser::parse(vector<Lexer::Token> const& theTokens)
{
	tokens = &theTokens;

	m_tree = make_unique<TTree>("program");
	program(*m_tree);

	if (m_trace < tokens->size() - 1)
		fatalError();

	return move(m_tree);
}

void Parser::fatalError()
{
	auto& token = (*tokens)[m_trace];
	printf("Unexpected token (l. %d, s. %d) : '%s'\n", token.line, token.symbol, token.value.c_str());

	system("pause");
	exit(EXIT_FAILURE);
}

void Parser::seek()
{
	if (m_index > tokens->size() - 1)
		throw "The end";

	token = &(*tokens)[m_index];

	raise();
}

Lexer::Token const* Parser::peek()
{
	if (m_index > tokens->size() - 1)
		return nullptr;

	return &(*tokens)[m_index];
}

void Parser::taste(LType type)
{
	seek();

	if (token->type != type)
		throw logic_error("The token type does not match");
}

void Parser::eat(LType type)
{
	seek();

	if (token->type != type)
		fatalError();
}

void Parser::raise()
{
	m_trace = max(m_trace, m_index);
	m_index++;
}

void Parser::compare(string const& value)
{
	if (token->value != value)
		fatalError();
}

void Parser::program(TTree& tree)
{
	while (true)
	{
		auto index = m_index;

		try { tree.push_back(move(*declaration().release())); }
		catch (...)
		{
			m_index = index;

			try { statement(tree); }
			catch (...)
			{
				m_index = index;

				try { block(tree); }
				catch (...)
				{
					m_index = index;

					return;
				}
			}
		}
	}
}

Parser::TTreePtr Parser::declaration()
{
	auto index = m_index;

	auto tree = make_unique<TTree>(TType::Declaration);

	try
	{
		typeSpec(*tree);

		eat(LType::Id);
		tree->emplace_back(token->value);

		if (auto t = peek())
		{
			auto& v = t->value;
			if (v == "=")
			{
				raise();

				tree->emplace_back(v);

				tree->push_back(move(*expression().release()));
			}
		}

		eat(LType::Separator);
		compare(";");

		return tree;
	}

	catch (exception&)
	{
		m_index = index;

		taste(LType::Keyword);
		if (token->value == "const")
		{
			tree->emplace_back(token->value);

			typeSpec(*tree);

			eat(LType::Id);
			tree->emplace_back(token->value);

			eat(LType::Operator);
			compare("=");
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(LType::Separator);
			compare(";");

			return tree;
		}
	}

	throw exception("");
}

void Parser::typeSpec(TTree& tree)
{
	taste(LType::Keyword);
	auto& v = token->value;
	if (v == "bool" || v == "float" || v == "int" || v == "string")
		tree.emplace_back(v);

	else
		throw exception("");
}

Parser::TTreePtr Parser::expression()
{
	auto tree = make_unique<TTree>(TType::Expression);

begin:
	logic1(*tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "||")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

void Parser::logic1(TTree& tree)
{
begin:
	logic2(tree);

	if (auto t = peek())
	{
		if (t->value == "&&")
		{
			raise();

			tree.emplace_back(t->value, VType::Operator);

			goto begin;
		}
	}
}

void Parser::logic2(TTree& tree)
{
begin:
	logic3(tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "==" || v == "!=")
		{
			raise();

			tree.emplace_back(v, VType::Operator);

			goto begin;
		}
	}
}

void Parser::logic3(TTree& tree)
{
begin:
	term(tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "<" || v == ">" || v == "<=" || v == ">=")
		{
			raise();

			tree.emplace_back(v, VType::Operator);

			goto begin;
		}
	}
}

void Parser::term(TTree& tree)
{
begin:
	factor(tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "+" || v == "-")
		{
			raise();

			tree.emplace_back(v, VType::Operator);

			goto begin;
		}
	}
}

void Parser::factor(TTree& tree)
{
begin:
	power(tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "*" || v == "/" || v == "%")
		{
			raise();

			tree.emplace_back(v, VType::Operator);

			goto begin;
		}
	}
}

void Parser::power(TTree& tree)
{
begin:
	operand(tree);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "^")
		{
			raise();

			tree.emplace_back(v, VType::Operator);

			goto begin;
		}
	}
}

void Parser::operand(TTree& tree)
{
	bool un = false;

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "-" || v == "!")
		{
			raise();

			if (v[0] == '-')
				tree.emplace_back("~", VType::Operator); // !!!!!!!!!!!!!!

			else
				tree.emplace_back("!", VType::Operator);

			un = true;
		}
	}

	if (auto t = peek())
	{
		if (t->value == "(")
		{
			raise();

			tree.emplace_back(t->value, VType::Operator);

			tree.push_back(move(*expression().release()));

			eat(LType::Operator);
			compare(")");
			tree.emplace_back(token->value, VType::Operator);

			return;
		}

		else if (t->type == LType::Id)
		{
			raise();

			tree.emplace_back(t->value, VType::Id);

			return;
		}

		else
		{
			literal(tree);

			return;
		}

		fatalError();
	}

	else if (un)
		fatalError();

	throw exception("");
}

void Parser::literal(TTree& tree)
{
	seek();

	if (token->type == LType::FloatLiteral)
		tree.emplace_back(token->value, VType::FloatLiteral);

	else if (token->type == LType::IntLiteral)
		tree.emplace_back(token->value, VType::IntLiteral);

	else if (token->type == LType::StringLiteral)
		tree.emplace_back(token->value, VType::StringLiteral);

	else if (token->value == "true" || token->value == "false")
		tree.emplace_back(token->value, VType::BoolLiteral);

	else
		fatalError();
}

void Parser::statement(TTree& tree)
{
	auto index = m_index;

	try { tree.push_back(move(*selStmt().release())); }
	catch (...)
	{
		m_index = index;

		try { tree.push_back(move(*iterStmt().release())); }
		catch (...)
		{
			m_index = index;

			try { tree.push_back(move(*printStmt().release())); }
			catch (...)
			{
				m_index = index;

				try { tree.push_back(move(*exprStmt().release())); }
				catch (...)
				{
					throw exception("");
				}
			}
		}
	}
}

Parser::TTreePtr Parser::selStmt()
{
	auto tree = make_unique<TTree>(TType::SelectionStatement);

	taste(LType::Keyword);
	if (token->value == "if")
	{
		tree->emplace_back(token->value);

		eat(LType::Operator);
		compare("(");
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(LType::Operator);
		compare(")");
		tree->emplace_back(token->value);

		block(*tree);

		if (auto t = peek())
		{
			auto& v = t->value;
			if (v == "else")
			{
				raise();

				tree->emplace_back(v);

				block(*tree);
			}
		}

		return tree;
	}

	throw exception("");
}

void Parser::block(TTree& tree)
{
	taste(LType::Separator);
	if (token->value == "{")
	{
		tree.emplace_back(token->value);

		program(tree);

		eat(LType::Separator);
		compare("}");
		tree.emplace_back(token->value);

		return;
	}

	throw exception("");
}

Parser::TTreePtr Parser::iterStmt()
{
	auto tree = make_unique<TTree>(TType::IterationStatement);

	taste(LType::Keyword);
	if (token->value == "while")
	{
		tree->emplace_back(token->value);

		eat(LType::Operator);
		compare("(");
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(LType::Operator);
		compare(")");
		tree->emplace_back(token->value);

		block(*tree);

		return tree;
	}

	throw exception("");
}

Parser::TTreePtr Parser::printStmt()
{
	auto tree = make_unique<TTree>(TType::PrintStatement);

	taste(LType::Keyword);
	if (token->value == "print")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(LType::Separator);
		compare(";");

		return tree;
	}

	throw exception("");
}

Parser::TTreePtr Parser::exprStmt()
{
	auto tree = make_unique<TTree>(TType::ExpressionStatement);

	taste(LType::Id);
	tree->emplace_back(token->value, VType::Id);

	eat(LType::Operator);
	compare("=");
	tree->emplace_back(token->value, VType::Operator);

	tree->push_back(move(*expression().release()));

	eat(LType::Separator);
	compare(";");

	return tree;
}