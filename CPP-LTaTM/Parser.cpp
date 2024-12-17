#include "Parser.h"

Parser::Parser(Lexer& theLexer) : 
	lexer(theLexer)
{

}

void Parser::parse()
{
	try 
	{
		tokens = &lexer.tokenize();
		
		m_tree = program();

		if (m_trace < tokens->size() - 1)
		{
			auto& token = (*tokens)[m_trace];

			printf("Unexpected token (l. %d, s. %d) : '%s'\n", token.line, token.symbol, token.value.c_str());
		}
		else
			m_tree->print();
	}
	catch (runtime_error e)
	{

	}
}


void Parser::seek()
{
	if (m_index > tokens->size() - 1)
		throw "";

	token = &(*tokens)[m_index];

	raise();
}

Token const& Parser::peek()
{
	if (m_index > tokens->size() - 1)
		throw "";

	return (*tokens)[m_index];
}

void Parser::eat(TokenType type)
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

Parser::STreePtr Parser::program()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::declaration()
{
	auto index = m_index;

	try
	{
		auto tree = make_unique<STree>(__func__);

		tree->push_back(move(*typeSpec().release()));

		eat(TokenType::Id);
		{
			STree _tree("id");
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

		eat(TokenType::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	catch (...)
	{
		auto tree = make_unique<STree>(__func__);

		m_index = index;

		eat(TokenType::Keyword);
		if (token->value == "const")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*typeSpec().release()));

			eat(TokenType::Id);
			{
				STree _tree("id");
				_tree.emplace_back(token->value);
				tree->push_back(move(_tree));
			}

			eat(TokenType::Operator);
			if (token->value == "=")
			{
				tree->emplace_back(token->value);
				tree->push_back(move(*expression().release()));

				eat(TokenType::Separator);
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

Parser::STreePtr Parser::typeSpec()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Keyword);
	auto& v = token->value;
	if (v == "bool" || v == "float" || v == "int" || v == "string")
	{
		tree->emplace_back(v);

		return tree;
	}

	throw runtime_error("");
}

Parser::STreePtr Parser::expression()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::logic1()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::logic2()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::logic3()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::term()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::factor()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::power()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::operand()
{
	auto tree = make_unique<STree>(__func__);

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

		eat(TokenType::Operator);
		if (token->value == ")")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	else if (tk.type == TokenType::Id)
	{
		raise();

		{
			STree _tree("id");
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

Parser::STreePtr Parser::literal()
{
	auto tree = make_unique<STree>(__func__);

	seek();

	if (token->type == TokenType::FloatLiteral)
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->type == TokenType::IntLiteral)
	{
		tree->emplace_back(token->value);

		return tree;
	}

	if (token->type == TokenType::StringLiteral)
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

Parser::STreePtr Parser::statement()
{
	auto tree = make_unique<STree>(__func__);

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

Parser::STreePtr Parser::selStmt()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Keyword);
	if (token->value == "if")
	{
		tree->emplace_back(token->value);

		eat(TokenType::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(TokenType::Operator);
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

Parser::STreePtr Parser::block()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Separator);
	if (token->value == "{")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*program().release()));

		eat(TokenType::Separator);
		if (token->value == "}")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}

Parser::STreePtr Parser::iterStmt()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Keyword);
	if (token->value == "while")
	{
		tree->emplace_back(token->value);

		eat(TokenType::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(TokenType::Operator);
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

Parser::STreePtr Parser::printStmt()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Keyword);
	if (token->value == "print")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(TokenType::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}

Parser::STreePtr Parser::exprStmt()
{
	auto tree = make_unique<STree>(__func__);

	eat(TokenType::Id);
	{
		STree _tree("id");
		_tree.emplace_back(token->value);
		tree->push_back(move(_tree));
	}

	eat(TokenType::Operator);
	if (token->value == "=")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(TokenType::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}