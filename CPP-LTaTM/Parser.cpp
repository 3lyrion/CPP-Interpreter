#include "Parser.h"

#include "Precompiled.h"

using Type = Shell::Token::Type;

Parser::Parser(Lexer& theLexer) : 
	lexer(theLexer)
{

}

Tree<Shell::Token> const& Parser::parse()
{
//	try 
//	{
		tokens = &lexer.tokenize();
		
		m_tree = program();

		if (m_trace < tokens->size() - 1)
		{
			auto& token = (*tokens)[m_trace];

			printf("Unexpected token (l. %d, s. %d) : '%s'\n", token.line, token.symbol, token.value.c_str());
		}
		else
		{
			m_tree->exclude(m_exclude);
			m_tree->print();
		}

		return *m_tree;
//	}
//	catch (runtime_error e)
//	{
//		cout << e.what() << '\n';

	//	return nullptr;
//	}
}


void Parser::seek()
{
	if (m_index > tokens->size() - 1)
		throw "";

	token = &(*tokens)[m_index];

	raise();
}

Lexer::Token const& Parser::peek()
{
	if (m_index > tokens->size() - 1)
		throw "";

	return (*tokens)[m_index];
}

void Parser::eat(Lexer::Lexer::Token::Type type)
{
	seek();

	if (token->type != type)
		throw runtime_error("");
}

void Parser::raise()
{
	m_trace = max(m_trace, m_index);
	m_index++;
}

Parser::TTreePtr Parser::program()
{
	auto tree = make_unique<TTree>(__func__);

	while (true)
	{
		auto index = m_index;

		try { tree->push_back(move(*declaration().release())); }
		catch (...)
		{
			m_index = index;

			try { tree->push_back(move(*statement().release())); }
			catch (...)
			{
				m_index = index;

				try { tree->push_back(move(*block().release())); }
				catch (...)
				{
					m_index = index;

					return tree;
				}
			}
		}
	}
}

Parser::TTreePtr Parser::declaration()
{
	auto index = m_index;

	try
	{
		auto tree = make_unique<TTree>(Type::Declaration);

		tree->push_back(move(*typeSpec().release()));

		eat(Lexer::Token::Type::Id);
		{
			TTree _tree("id");
			_tree.emplace_back(token->value);
			tree->push_back(move(_tree));
		}

		auto& v = peek().value;
		if (v == "=")
		{
			raise();

			tree->emplace_back(v);
			tree->push_back(move(*expression().release()));
		}

		eat(Lexer::Token::Type::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	catch (...)
	{
		auto tree = make_unique<TTree>(Type::Declaration);

		m_index = index;

		eat(Lexer::Token::Type::Keyword);
		if (token->value == "const")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*typeSpec().release()));

			eat(Lexer::Token::Type::Id);
			{
				TTree _tree("id");
				_tree.emplace_back(token->value);
				tree->push_back(move(_tree));
			}

			eat(Lexer::Token::Type::Operator);
			if (token->value == "=")
			{
				tree->emplace_back(token->value);
				tree->push_back(move(*expression().release()));

				eat(Lexer::Token::Type::Separator);
				if (token->value == ";")
				{
					tree->emplace_back(token->value);

					return tree;
				}
			}
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::typeSpec()
{
	auto tree = make_unique<TTree>(__func__);

	eat(Lexer::Token::Type::Keyword);
	auto& v = token->value;
	if (v == "bool" || v == "float" || v == "int" || v == "string")
	{
		tree->emplace_back(v);

		return tree;
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::expression()
{
	auto tree = make_unique<TTree>(Type::Expression);

begin:
	tree->push_back(move(*logic1().release()));

	auto& v = peek().value;
	if (v == "||")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::logic1()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*logic2().release()));

	auto& v = peek().value;
	if (v == "&&")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::logic2()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*logic3().release()));

	auto& v = peek().value;
	if (v == "==" || v == "!=")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::logic3()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*term().release()));

	auto& v = peek().value;
	if (v == "<" || v == ">" || v == "<=" || v == ">=")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::term()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*factor().release()));

	auto& v = peek().value;
	if (v == "+" || v == "-")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::factor()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*power().release()));

	auto& v = peek().value;
	if (v == "*" || v == "/" || v == "%")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::power()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*operand().release()));

	auto& v = peek().value;
	if (v == "^")
	{
		raise();

		tree->emplace_back(v);

		goto begin;
	}

	return tree;
}

Parser::TTreePtr Parser::operand()
{
	auto tree = make_unique<TTree>(__func__);

	auto& v = peek().value;
	if (v == "-" || v == "!")
	{
		raise();

		tree->emplace_back(v);
	}

	auto& tk = peek();

	if (tk.value == "(")
	{
		raise();

		tree->emplace_back(tk.value);

		tree->push_back(move(*expression().release()));

		eat(Lexer::Token::Type::Operator);
		if (token->value == ")")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	else if (tk.type == Lexer::Token::Type::Id)
	{
		raise();

		{
			TTree _tree("id");
			_tree.emplace_back(tk.value);
			tree->push_back(move(_tree));
		}

		return tree;
	}

	else
	{
		tree->push_back(move(*literal().release()));

		return tree;
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::literal()
{
	auto tree = make_unique<TTree>(__func__);

	seek();

	if (token->type == Lexer::Token::Type::FloatLiteral)
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->type == Lexer::Token::Type::IntLiteral)
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->type == Lexer::Token::Type::StringLiteral)
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->value == "true")
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->value == "false")
	{
		tree->emplace_back(token->value);

		return tree;
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::statement()
{
	auto tree = make_unique<TTree>(__func__);

	auto index = m_index;

	try { tree->push_back(move(*selStmt().release())); }
	catch (...)
	{
		m_index = index;

		try { tree->push_back(move(*iterStmt().release())); }
		catch (...)
		{
			m_index = index;

			try { tree->push_back(move(*printStmt().release())); }
			catch (...)
			{
				m_index = index;

				try { tree->push_back(move(*exprStmt().release())); }
				catch (...)
				{
					throw runtime_error("");
				}
			}
		}
	}

	return tree;
}

Parser::TTreePtr Parser::selStmt()
{
	auto tree = make_unique<TTree>(Type::SelectionStatement);

	eat(Lexer::Token::Type::Keyword);
	if (token->value == "if")
	{
		tree->emplace_back(token->value);

		eat(Lexer::Token::Type::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(Lexer::Token::Type::Operator);
			if (token->value == ")")
			{
				tree->emplace_back(token->value);

				tree->push_back(move(*block().release()));

				auto& v = peek().value;
				if (v == "else")
				{
					raise();

					tree->emplace_back(v);

					tree->push_back(move(*block().release()));
				}

				return tree;
			}
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::block()
{
	auto tree = make_unique<TTree>(__func__);

	eat(Lexer::Token::Type::Separator);
	if (token->value == "{")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*program().release()));

		eat(Lexer::Token::Type::Separator);
		if (token->value == "}")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::iterStmt()
{
	auto tree = make_unique<TTree>(Type::IterationStatement);

	eat(Lexer::Token::Type::Keyword);
	if (token->value == "while")
	{
		tree->emplace_back(token->value);

		eat(Lexer::Token::Type::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(Lexer::Token::Type::Operator);
			if (token->value == ")")
			{
				tree->emplace_back(token->value);

				tree->push_back(move(*block().release()));

				return tree;
			}
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::printStmt()
{
	auto tree = make_unique<TTree>(Type::PrintStatement);

	eat(Lexer::Token::Type::Keyword);
	if (token->value == "print")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(Lexer::Token::Type::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::exprStmt()
{
	auto tree = make_unique<TTree>(Type::ExpressionStatement);

	eat(Lexer::Token::Type::Id);
	{
		TTree _tree("id");
		_tree.emplace_back(token->value);
		tree->push_back(move(_tree));
	}

	eat(Lexer::Token::Type::Operator);
	if (token->value == "=")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(Lexer::Token::Type::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}