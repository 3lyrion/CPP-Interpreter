#include "Parser.h"

void Parser::parse(queue<Token> const& tokens)
{
	m_tokens = tokens;

	try { program(); cout << "УСПЕХ\n"; }
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

void Parser::nextToken()
{
	if (m_tokens.empty())
		throw "Конец";

	m_token = m_tokens.front();
	m_tokens.pop();
}

Token& Parser::peek()
{
	return m_tokens.front();
}

void Parser::eat(TokenType type)
{
	if (m_token.type == type)
		nextToken();

	else throw runtime_error("Неожиданный токен");
}

void Parser::seek(TokenType type)
{
	nextToken();
	eat(type);
}

void Parser::program()
{
	while (true)
	{
		try { declaration(); }
		catch (...)
		{
			try { statement(); }
			catch (...)
			{
				try { block(); }
				catch (...)
				{
					return;
				}
			}
		}
	}
}

void Parser::declaration()
{
	try
	{
		typeSpec();

		seek(TokenType::Id);

		auto& tk = peek();
		if (tk.value == "=")
		{
			nextToken();

			expression();
		}

		seek(TokenType::Separator);
		if (m_token.value == ";")
			return;
	}

	catch (...)
	{
		seek(TokenType::Keyword);
		if (m_token.value == "const")
		{
			typeSpec();

			seek(TokenType::Id);

			seek(TokenType::Operator);
			if (m_token.value == "=")
			{
				expression();

				seek(TokenType::Separator);
				if (m_token.value == ";")
					return;
			}
		}
	}

	throw runtime_error("");
}

void Parser::typeSpec()
{
	seek(TokenType::Keyword);
	auto& v = m_token.value;
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
		nextToken();

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
		nextToken();

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
		nextToken();

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
		nextToken();

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
		nextToken();

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
		nextToken();

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
		nextToken();

		goto begin;
	}

	return;
}

void Parser::operand()
{
	auto& v = peek().value;
	if (v == "-" || v == "!")
	{
		nextToken();
	}

	nextToken();

	auto t = m_token.type;

	if (m_token.value == "(")
	{
		expression();

		seek(TokenType::Separator);
		if (m_token.value == ")")
			return;
	}

	else if (t == TokenType::Id)
	{
		return;
	}

	else if (t == TokenType::FloatLiteral || t == TokenType::IntLiteral || t == TokenType::StringLiteral)
	{
		return;
	}

	throw runtime_error("");
}

void Parser::literal()
{
	nextToken();

	if (m_token.type == TokenType::FloatLiteral)
	{
		return;
	}

	if (m_token.type == TokenType::IntLiteral)
	{
		return;
	}

	if (m_token.type == TokenType::StringLiteral)
	{
		return;
	}

	if (m_token.value == "true")
	{
		return;
	}

	if (m_token.value == "false")
	{
		return;
	}

	throw runtime_error("");
}

void Parser::statement()
{
	try { selStmt(); }
	catch (...)
	{
		try { iterStmt(); }
		catch (...)
		{
			try { printStmt(); }
			catch (...)
			{
				try { exprStmt(); }
				catch (...)
				{
		
				}
			}
		}
	}

	throw runtime_error("");
}

void Parser::selStmt()
{
	seek(TokenType::Keyword);
	if (m_token.value == "if")
	{
		seek(TokenType::Operator);
		if (m_token.value == "(")
		{
			expression();

			seek(TokenType::Operator);
			if (m_token.value == ")")
			{
				block();

				auto& v = peek().value;
				if (v == "else")
				{
					nextToken();

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
	seek(TokenType::Separator);
	if (m_token.value == "{")
	{
		program();

		seek(TokenType::Separator);
		if (m_token.value == "}")
		{
			return;
		}
	}

	throw runtime_error("");
}

void Parser::iterStmt()
{
	seek(TokenType::Keyword);
	if (m_token.value == "while")
	{
		seek(TokenType::Separator);
		if (m_token.value == "(")
		{
			expression();

			seek(TokenType::Separator);
			if (m_token.value == ")")
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
	seek(TokenType::Keyword);
	if (m_token.value == "print")
	{
		expression();

		seek(TokenType::Separator);
		if (m_token.value == ";")
		{
			return;
		}
	}

	throw runtime_error("");
}

void Parser::exprStmt()
{
	seek(TokenType::Id);

	seek(TokenType::Operator);
	if (m_token.value == "=")
	{
		expression();

		seek(TokenType::Separator);
		if (m_token.value == ";")
		{
			return;
		}
	}

	throw runtime_error("");
}