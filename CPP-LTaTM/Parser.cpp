#include "Parser.h"

#include "Precompiled.h"

using LType = Lexer::Token::Type;
using TType = Shell::Token::Type;
using VType = Shell::Token::Value::Type;

Parser::Parser(Lexer& theLexer) : 
	lexer(theLexer)
{

}

Parser::TTreePtr Parser::parse()
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
//			m_tree->print();
		}

		return move(m_tree);
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

Lexer::Token const* Parser::peek()
{
	if (m_index > tokens->size() - 1)
		return nullptr;

	return &(*tokens)[m_index];
}

void Parser::eat(LType type)
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
		auto tree = make_unique<TTree>(TType::Declaration);

		tree->push_back(move(*typeSpec().release()));

		eat(LType::Id);
		{
			TTree _tree("id");
			_tree.emplace_back(token->value);
			tree->push_back(move(_tree));
		}

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
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	catch (...)
	{
		auto tree = make_unique<TTree>(TType::Declaration);

		m_index = index;

		eat(LType::Keyword);
		if (token->value == "const")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*typeSpec().release()));

			eat(LType::Id);
			{
				TTree _tree("id");
				_tree.emplace_back(token->value);
				tree->push_back(move(_tree));
			}

			eat(LType::Operator);
			if (token->value == "=")
			{
				tree->emplace_back(token->value);
				tree->push_back(move(*expression().release()));

				eat(LType::Separator);
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

	eat(LType::Keyword);
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
	auto tree = make_unique<TTree>(TType::Expression);

begin:
	tree->push_back(move(*logic1().release()));

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

Parser::TTreePtr Parser::logic1()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*logic2().release()));

	if (auto t = peek())
	{
		if (t->value == "&&")
		{
			raise();

			tree->emplace_back(t->value, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::logic2()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*logic3().release()));

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "==" || v == "!=")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::logic3()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*term().release()));

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "<" || v == ">" || v == "<=" || v == ">=")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::term()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*factor().release()));

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "+" || v == "-")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::factor()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*power().release()));

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "*" || v == "/" || v == "%")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::power()
{
	auto tree = make_unique<TTree>(__func__);

begin:
	tree->push_back(move(*operand().release()));

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "^")
		{
			raise();

			tree->emplace_back(v, VType::Operator);

			goto begin;
		}
	}

	return tree;
}

Parser::TTreePtr Parser::operand()
{
	auto tree = make_unique<TTree>(__func__);

	if (auto t = peek())
	{
		auto& v = t->value;
		if (v == "-" || v == "!")
		{
			raise();

			if (v[0] == '-')
				tree->emplace_back("~", VType::Operator); // !!!!!!!!!!!!!!

			else
				tree->emplace_back("!", VType::Operator);
		}
	}

	if (auto t = peek())
	{
		if (t->value == "(")
		{
			raise();

			tree->emplace_back(t->value, VType::Operator);

			tree->push_back(move(*expression().release()));

			eat(LType::Operator);
			if (token->value == ")")
			{
				tree->emplace_back(token->value, VType::Operator);

				return tree;
			}
		}

		else if (t->type == LType::Id)
		{
			raise();

			{
				TTree _tree("id");
				_tree.emplace_back(t->value, VType::Id);
				tree->push_back(move(_tree));
			}

			return tree;
		}

		else
		{
			tree->push_back(move(*literal().release()));

			return tree;
		}
	}

	throw runtime_error("");
}

Parser::TTreePtr Parser::literal()
{
	auto tree = make_unique<TTree>(__func__);

	seek();

	if (token->type == LType::FloatLiteral)
	{
		tree->emplace_back(token->value, VType::FloatLiteral);

		return tree;
	}

	if (token->type == LType::IntLiteral)
	{
		tree->emplace_back(token->value, VType::IntLiteral);

		return tree;
	}

	if (token->type == LType::StringLiteral)
	{
		tree->emplace_back(token->value, VType::StringLiteral);

		return tree;
	}

	if (token->value == "true" || token->value == "false")
	{
		tree->emplace_back(token->value, VType::BoolLiteral);

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
	auto tree = make_unique<TTree>(TType::SelectionStatement);

	eat(LType::Keyword);
	if (token->value == "if")
	{
		tree->emplace_back(token->value);

		eat(LType::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(LType::Operator);
			if (token->value == ")")
			{
				tree->emplace_back(token->value);

				tree->push_back(move(*block().release()));

				if (auto t = peek())
				{
					auto& v = t->value;
					if (v == "else")
					{
						raise();

						tree->emplace_back(v);

						tree->push_back(move(*block().release()));
					}
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

	eat(LType::Separator);
	if (token->value == "{")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*program().release()));

		eat(LType::Separator);
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
	auto tree = make_unique<TTree>(TType::IterationStatement);

	eat(LType::Keyword);
	if (token->value == "while")
	{
		tree->emplace_back(token->value);

		eat(LType::Operator);
		if (token->value == "(")
		{
			tree->emplace_back(token->value);

			tree->push_back(move(*expression().release()));

			eat(LType::Operator);
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
	auto tree = make_unique<TTree>(TType::PrintStatement);

	eat(LType::Keyword);
	if (token->value == "print")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(LType::Separator);
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
	auto tree = make_unique<TTree>(TType::ExpressionStatement);

	eat(LType::Id);
	{
		TTree _tree("id");
		_tree.emplace_back(token->value);
		tree->push_back(move(_tree));
	}

	eat(LType::Operator);
	if (token->value == "=")
	{
		tree->emplace_back(token->value);

		tree->push_back(move(*expression().release()));

		eat(LType::Separator);
		if (token->value == ";")
		{
			tree->emplace_back(token->value);

			return tree;
		}
	}

	throw runtime_error("");
}