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

		program();
		
		cout << token->value << "\n";
	}
	catch (string& e)
	{
		if (e == "Конец")
		{
			cout << "УСПЕХ\n";
		}

		else
		{
			cout << "Неожиданный токен\n";
		}
	}
}

void Parser::seek(size_t offset)
{
	if (m_index > tokens->size() - 1)
		throw "Конец";

	token = &(*tokens)[m_index];

	m_index += offset;
}

Token const& Parser::peek()
{
	if (m_index > tokens->size() - 1)
		throw "Конец";

	return (*tokens)[m_index];
}

void Parser::eat(TokenType type)
{
	seek();

	if (token->type != type)
		throw runtime_error("Неожиданный токен");
}

void Parser::program()
{
	if (m_tree.empty())
		m_tree.init(__func__);
	else
		m_tree.emplace_back(__func__);

	while (true)
	{
		auto index = m_index;

		try { declaration(); }
		catch (...)
		{
			m_index = index;

			try { statement(); }
			catch (...)
			{
				m_index = index;

				try { block(); }
				catch (...)
				{
					m_index = index;

					return;
				}
			}
		}
	}
}

void Parser::declaration()
{


	auto index = m_index;

	try
	{
		typeSpec();

		eat(TokenType::Id);

		auto& tk = peek();
		if (tk.value == "=")
		{
			m_index++;

			expression();
		}

		eat(TokenType::Separator);
		if (token->value == ";")
			return;
	}

	catch (...)
	{
		m_index = index;

		eat(TokenType::Keyword);
		if (token->value == "const")
		{
			typeSpec();

			eat(TokenType::Id);

			eat(TokenType::Operator);
			if (token->value == "=")
			{
				expression();

				eat(TokenType::Separator);
				if (token->value == ";")
					return;
			}
		}
	}

	throw runtime_error("");
}

void Parser::typeSpec()
{
	eat(TokenType::Keyword);
	auto& v = token->value;
	if (v == "bool" || v == "float" || v == "int" || v == "string")
		return;

	throw runtime_error("");
}

void Parser::expression()
{
begin:
	logic1();

	auto& v = peek().value;
	if (v == "||")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::logic1()
{
begin:
	logic2();

	auto& v = peek().value;
	if (v == "&&")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::logic2()
{
begin:
	logic3();

	auto& v = peek().value;
	if (v == "==" || v == "!=")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::logic3()
{
begin:
	term();

	auto& v = peek().value;
	if (v == "<" || v == ">" || v == "<=" || v == ">=")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::term()
{
begin:
	factor();

	auto& v = peek().value;
	if (v == "+" || v == "-")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::factor()
{
begin:
	power();

	auto& v = peek().value;
	if (v == "*" || v == "/" || v == "%")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::power()
{
begin:
	operand();

	auto& v = peek().value;
	if (v == "^")
	{
		m_index++;

		goto begin;
	}

	return;
}

void Parser::operand()
{
	auto& v = peek().value;
	if (v == "-" || v == "!")
	{
		m_index++;
	}

	auto& tk = peek();

	if (tk.value == "(")
	{
		m_index++;

		expression();

		eat(TokenType::Operator);
		if (token->value == ")")
			return;
	}

	else if (tk.type == TokenType::Id)
	{
		m_index++;

		return;
	}

	else
	{
		literal();
	}
}

void Parser::literal()
{
	seek();

	if (token->type == TokenType::FloatLiteral)
	{
		return;
	}

	if (token->type == TokenType::IntLiteral)
	{
		return;
	}

	if (token->type == TokenType::StringLiteral)
	{
		return;
	}

	if (token->value == "true")
	{
		return;
	}

	if (token->value == "false")
	{
		return;
	}

	throw runtime_error("");
}

void Parser::statement()
{
	auto index = m_index;

	try { selStmt(); }
	catch (...)
	{
		m_index = index;

		try { iterStmt(); }
		catch (...)
		{
			m_index = index;

			try { printStmt(); }
			catch (...)
			{
				m_index = index;

				try { exprStmt(); }
				catch (...)
				{
					throw runtime_error("");
				}
			}
		}
	}
}

void Parser::selStmt()
{
	eat(TokenType::Keyword);
	if (token->value == "if")
	{
		eat(TokenType::Operator);
		if (token->value == "(")
		{
			expression();

			eat(TokenType::Operator);
			if (token->value == ")")
			{
				block();

				auto& v = peek().value;
				if (v == "else")
				{
					m_index++;

					block();
				}

				return;
			}
		}
	}

	throw runtime_error("");
}

void Parser::block()
{
	eat(TokenType::Separator);
	if (token->value == "{")
	{
		program();

		eat(TokenType::Separator);
		if (token->value == "}")
		{
			return;
		}
	}

	throw runtime_error("");
}

void Parser::iterStmt()
{
	eat(TokenType::Keyword);
	if (token->value == "while")
	{
		eat(TokenType::Operator);
		if (token->value == "(")
		{
			expression();

			eat(TokenType::Operator);
			if (token->value == ")")
			{
				block();

				return;
			}
		}
	}

	throw runtime_error("");
}

void Parser::printStmt()
{
	eat(TokenType::Keyword);
	if (token->value == "print")
	{
		expression();

		eat(TokenType::Separator);
		if (token->value == ";")
		{
			return;
		}
	}

	throw runtime_error("");
}

void Parser::exprStmt()
{
	eat(TokenType::Id);

	eat(TokenType::Operator);
	if (token->value == "=")
	{
		expression();

		eat(TokenType::Separator);
		if (token->value == ";")
		{
			return;
		}
	}

	throw runtime_error("");
}