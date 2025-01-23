#include <Lexer.h>

#include <regex>

using TkType = Lexer::Token::Type;

const regex REGEX_ID     (R"([A-Za-z_]+[A-Za-z0-9_]*)",	regex_constants::ECMAScript | regex_constants::nosubs);
const regex REGEX_INT    (R"(-?([1-9][0-9]*|0))",		regex_constants::ECMAScript | regex_constants::nosubs);
const regex REGEX_FLOAT  (R"(-?[0-9]*\.[0-9]*)",		regex_constants::ECMAScript | regex_constants::nosubs);
const regex REGEX_STRING (R"("[^"\n\r\t]*")",			regex_constants::ECMAScript | regex_constants::nosubs);

const unordered_set<string> KEYWORDS =
{
	"bool",
	"float",
	"int",
	"string",
	"if",
	"else",
	"while",
	"const",
	"false",
	"true",
	"print"
};

const unordered_set<string> OPERATORS =
{
	"+",
	"-",
	"*",
	"/",
	"^",
	"%",
	"!",
	"=",
	"(",
	")",
	"<",
	">",
	"==",
	">=",
	"<=",
	"!=",
	"&&",
	"||"
};

const unordered_set<string> SEPARATORS =
{
	"{",
	"}",
	";"
};

vector<Lexer::Token> const& Lexer::tokenize(fs::path const& path)
{
	if (fs::exists(path) && path.has_filename())
	{
		ifstream		src_file(path);
		ostringstream	oss;

		oss << src_file.rdbuf();
		m_src = oss.str();
		src_file.close();
	}

	else
		throw exception("Cannot open the file");

	m_srcLength = m_src.length();

	while (eatNextToken());

	return m_tokens;
}
		
bool Lexer::eatNextToken()
{
	skipWhitespace();
	skipComment();

	string symb = string{} + m_char;
	string word = symb + peek();

	if (OPERATORS.contains(word))
	{
		m_tokens.emplace_back(TkType::Operator, word, m_curLine, m_curSymbol);
		advance(2);
		return true;
	}

	if (OPERATORS.contains(symb))
	{
		m_tokens.emplace_back(TkType::Operator, symb, m_curLine, m_curSymbol);
		advance();
		return true;
	}

	if (SEPARATORS.contains(symb))
	{
		m_tokens.emplace_back(TkType::Separator, symb, m_curLine, m_curSymbol);
		advance();
		return true;
	}

	smatch result{};
	string tk_value;

	regex_search(m_src.cbegin() + m_pos, m_src.cend(), result, REGEX_ID, regex_constants::match_continuous);
	if (!result.empty())
	{
		tk_value = result[0];

		if (KEYWORDS.contains(tk_value))
			m_tokens.emplace_back(TkType::Keyword, tk_value, m_curLine, m_curSymbol);
				
		else
			m_tokens.emplace_back(TkType::Id, tk_value, m_curLine, m_curSymbol);

		advance(tk_value.size());

		return true;
	}

	regex_search(m_src.cbegin() + m_pos, m_src.cend(), result, REGEX_FLOAT, regex_constants::match_continuous);
	if (!result.empty())
	{
		tk_value = result[0];
		m_tokens.emplace_back(TkType::FloatLiteral, tk_value, m_curLine, m_curSymbol);

		advance(tk_value.size());

		return true;
	}
	
	regex_search(m_src.cbegin() + m_pos, m_src.cend(), result, REGEX_INT, regex_constants::match_continuous);
	if (!result.empty())
	{
		tk_value = result[0];
		m_tokens.emplace_back(TkType::IntLiteral, tk_value, m_curLine, m_curSymbol);

		advance(tk_value.size());

		return true;
	}

	regex_search(m_src.cbegin() + m_pos, m_src.cend(), result, REGEX_STRING, regex_constants::match_continuous);
	if (!result.empty())
	{
		tk_value = result[0];
		m_tokens.emplace_back(TkType::StringLiteral, tk_value.substr(1, tk_value.size() - 2ull), m_curLine, m_curSymbol);

		advance(tk_value.size());

		return true;
	}

	return false;
}

size_t Lexer::advance(size_t n)
{
	if (n == 0ull)
		return {};

	size_t advanced = 0;

	while (m_pos < m_srcLength && advanced < n)
	{
		++advanced;

		if (m_char == '\n')
		{
			++m_curLine;
			m_curSymbol = 0u;
		}

		m_char = m_src[++m_pos];
		++m_curSymbol;
	}

	return advanced;

}

char Lexer::peek() const
{
	auto npos = m_pos + 1ull;

	if (npos < m_srcLength)
		return m_src[npos];

	return '\0';
}

bool Lexer::hasCharsLeft() const
{
	return m_char != '\0';
}

void Lexer::skipWhitespace()
{
	do
	{
		if (!iswspace(m_char))
			break;
		
	} while (advance());
}

void Lexer::skipComment()
{
	if (m_char != '/')
		return;

	advance();

	auto st_line = m_curLine;
	bool asterisk_met_first{};
	bool asterisk_met_sec{};

	if (m_char == '*')
	{
		advance();

		while (hasCharsLeft() && !asterisk_met_sec)
		{
			if (m_char == '*')
			{
				if (!asterisk_met_first)
					asterisk_met_first = true;

				else if (!asterisk_met_sec && peek() == '/')
					asterisk_met_sec = true;
			}

			else
				asterisk_met_first = false;

			advance();
		}

		if (!asterisk_met_sec)
		{
			ostringstream msg;

			msg << "Syntax error in line "
				<< m_curLine
				<< ": multiline comment not closed";

			throw exception(msg.str().c_str());
			
		}
	}
	
	else
	{
		while (hasCharsLeft() && (m_char != '\n'))
			advance();
	}

	skipWhitespace();
}