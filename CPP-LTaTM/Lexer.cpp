#include "Lexer.hpp"

Lexer::Lexer(filesystem::path srcPath) :
	m_srcPath(fs::absolute(srcPath))
{
	if (fs::exists(m_srcPath) && m_srcPath.has_filename())
	{
		ifstream srcFile(m_srcPath);

		ostringstream sstr;
		sstr << srcFile.rdbuf();
		m_src = sstr.str();
		srcFile.close();
	}

	m_srcLength = m_src.length();

	m_keywords.insert("if");
	m_keywords.insert("else");
	m_keywords.insert("while");
	m_keywords.insert("int");
	m_keywords.insert("float");
	m_keywords.insert("bool");
	m_keywords.insert("string");
	m_keywords.insert("const");
	m_keywords.insert("true");
	m_keywords.insert("false");
	m_keywords.insert("print");

	m_operators.insert("+");
	m_operators.insert("-");
	m_operators.insert("*");
	m_operators.insert("/");
	m_operators.insert("^");
	m_operators.insert("%");
	m_operators.insert("!");
	m_operators.insert("=");
	m_operators.insert("(");
	m_operators.insert(")");
	m_operators.insert("<");
	m_operators.insert(">");
	m_operators.insert("==");
	m_operators.insert(">=");
	m_operators.insert("<=");
	m_operators.insert("!=");
	m_operators.insert("&&");
	m_operators.insert("||");

	m_compounds.insert("==");
	m_compounds.insert(">=");
	m_compounds.insert("<=");
	m_compounds.insert("!=");
	m_compounds.insert("/*");
	m_compounds.insert("*/");
	m_compounds.insert("//");
//	m_compounds.insert("&&");
//	m_compounds.insert("||");

	initDFA();
}

vector<Token> const& Lexer::tokenize()
{
	bool parsingOneLineComment = false;
	bool parsingMultiLineComment = false;
	bool asteriskMet = false;

	bool startedString = false;

	string lexem;
	char c;
	int separatorSwitch = 2;

	try
	{
	while (m_pos < m_srcLength)
	{
		++m_currentChar;
		c = m_src[m_pos];

		if (c == '\n')
		{
			m_currentChar = 0;
			++m_currentLine;
		}

		if (parsingOneLineComment)
		{
			if (c == '\n')
			{
				parsingOneLineComment = false;
				lexem = "";
			}

			++m_pos;
			
		}

		else if (parsingMultiLineComment)
		{
			if (c == '*')
				asteriskMet = true;

			else if (c == '/')
			{
				if (asteriskMet)
				{
					parsingMultiLineComment = false;
					asteriskMet = false;
				}
				
				else asteriskMet = false;
			}
			
			else asteriskMet = false;

			++m_pos;

			if (m_pos >= m_srcLength)
				throw "Многострочный комментарий, открытый на стр. " + to_string(m_commentStartLine) + ", не был закрыт";
			
		} 
		
		else if (c == '"')
		{
			startedString = !startedString;
			lexem += c;
			++m_pos;
		}
		
		else if (startedString)
		{
			if (c == '\n')
				throw "Неверный строковый символ в стр. " + to_string(m_currentLine - 1);

			lexem += c;
			++m_pos;
		}
		
		else
		{
			bool isNum = isNumber(c);
			bool isWs = isNum ? false : isWhitespace(c);
			bool isOp = isWs ? false : isOperator(c);
			bool isSep = isOp ? true : isSeparator(c);

			if (separatorSwitch == 0 || separatorSwitch == 1)
			{
				if (!isWs && !isSep)
				{
					if (!isWs)
						lexem += c;

					goto end;
				}

				if (m_keywords.contains(lexem))
					m_tokens.emplace_back(TokenType::Keyword, lexem);

				else if (m_regex_float.Process(lexem))
					m_tokens.emplace_back(TokenType::FloatLiteral, lexem);

				else if (m_regex_int.Process(lexem))
					m_tokens.emplace_back(TokenType::IntLiteral, lexem);

				else if (m_regex_id.Process(lexem))
					m_tokens.emplace_back(TokenType::Id, lexem);

				else if (m_regex_string.Process(lexem))
				{
					if (lexem.length() == 2)
						m_tokens.emplace_back(TokenType::StringLiteral, "");

					else
						m_tokens.emplace_back(TokenType::StringLiteral, string(lexem.begin() + 1, lexem.end() - 1));
				}
						
				lexem = "";

				if (isSep)
				{
					separatorSwitch = 2;
					lexem += c;

					goto end;
				}
			}

			if (separatorSwitch == 0 || separatorSwitch == 2)
			{
				if (lexem == "{") 
				{
					m_tokens.emplace_back(TokenType::Separator, "{");
					lexem = "";

				} 
				
				else if (lexem == "}")
				{
					m_tokens.emplace_back(TokenType::Separator, "}");
					lexem = "";

				} 
				
				else if (lexem == ";")
				{
					m_tokens.emplace_back(TokenType::Separator, ";");
					lexem = "";
				}

				else if (lexem == "//")
				{
					parsingOneLineComment = true;

					lexem = "";

					goto end;
				}

				else if (lexem == "/*")
				{
					m_commentStartLine = m_currentLine - 1;
					parsingMultiLineComment = true;

					if (c == '*')
						asteriskMet = true;

					lexem = "";

					goto end;
				}

				else if (m_operators.contains(lexem) && !m_compounds.contains(lexem + c))
				{
					m_tokens.emplace_back(TokenType::Operator, lexem);
					lexem = "";
				}

				if (isSep)
				{
					lexem += c;

					goto end;
				} 

				separatorSwitch = 1;

				if (!isWs) lexem += c;						
			}

		end:
			/*
			if (isWs)
			{
				bool noError = false;

				for (auto& kw : m_keywords)
				{
					if (kw.find(lexem) != string::npos)
					{
						noError = true;

						break;
					}
				}

				if (!noError)
					throw string("Запрещённый символ ") + lexem;
			}
			*/
			++m_pos;
			
		}
	}
	} // try

	catch (string& error)
	{
		printf("Ошибка (стр. %zu, с. %zu) : ", m_currentLine, m_currentChar);
		cerr << error << '\n';

		throw runtime_error("");
	}

	return m_tokens;
	
}

bool Lexer::isWhitespace(char c) {
	
	return (c == ' ' || c == '\n' || c == '\t' || c == '\b' || c == '\r');
	
}
	
bool  Lexer::isSeparator(char c) {
	
	return (c == '(' || c == ')' || c == '{' || c == '}' || c == ';') || isOperator(c);
				
	
}

bool Lexer::isOperator(char c) {
	
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' ||
			c == '>' || c == '<' || c == '!' || c == '=' || c == '&' || c == '|');
}

bool Lexer::isNumber(char c) {
	
	return (c >= '0' && c <= '9');
	
}