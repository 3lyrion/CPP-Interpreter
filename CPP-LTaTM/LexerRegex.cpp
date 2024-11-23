#include "Lexer.hpp"

DFA::TransitionPred trFunc_isNumeric = [](char c) -> bool {

	if (c >= '0' && c <= '9')
		return true;

	return false;

};

DFA::TransitionPred trFunc_isMinus = [](char c) -> bool {

	if (c == '-')
		return true;

	return false;

};

DFA::TransitionPred trFunc_isDot = [](char c) -> bool {

	if (c == '.')
		return true;

	return false;

};

DFA::TransitionPred trFunc_isAlphaLatin = [](char c) -> bool {

	if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z')
		return true;

	return false;

};

DFA::TransitionPred trFunc_isAlphaCyrillic = [](char c) -> bool {
	
	return isalpha(c, locale("ru_RU")); 

};

DFA::TransitionPred trFunc_isUnderscore = [](char c) -> bool {
	
	return (c == '_');

};

DFA::TransitionPred trFunc_isIdBegin = [](char c) -> bool {
	
	return (trFunc_isAlphaLatin(c) || trFunc_isUnderscore(c));

};

DFA::TransitionPred trFunc_isIdRest = [](char c) -> bool {
	
	return (trFunc_isAlphaLatin(c) || trFunc_isUnderscore(c) || trFunc_isNumeric(c));

};

DFA::TransitionPred trFunc_isSingleQuote = [](char c) -> bool {
	
	return (c == '\'');

};

DFA::TransitionPred trFunc_isDoubleQuote = [](char c) -> bool {
	
	return (c == '"');

};

DFA::TransitionPred trFunc_isBackslash = [](char c) -> bool {
	
	return (c == '\\');

};

DFA::TransitionPred trFunc_notBackslashAndSingleQuote = [](char c) -> bool {
	
	return (c != '\\' && c != '\'');

};

DFA::TransitionPred trFunc_notBackslashAndDoubleQuote = [](char c) -> bool {
	
	return (c != '\\' && c != '"');

};

DFA::TransitionPred trFunc_isEscapeSingle = [](char c) -> bool {
	
	return (c == '\\' || c == '\'' || c == 'b' || c == 'n' || c == 'r' || c == 't');

};

DFA::TransitionPred trFunc_isEscapeDouble = [](char c) -> bool {
	
	return (c == '\\' || c == '"' || c == 'b' || c == 'n' || c == 'r' || c == 't');

};

void Lexer::initDFA()
{
	m_regex_id = DFA(2, 0, {1});
	m_regex_id.SetTransition({0, 1}, trFunc_isIdBegin);
	m_regex_id.SetTransition({1, 1}, trFunc_isIdRest);

	m_regex_int = DFA(3, 0, {2});
	m_regex_int.SetTransition({0, 1}, trFunc_isMinus);
	m_regex_int.SetTransition({0, 2}, trFunc_isNumeric);
	m_regex_int.SetTransition({1, 2}, trFunc_isNumeric);
	m_regex_int.SetTransition({2, 2}, trFunc_isNumeric);

	m_regex_float = DFA(4, 0, {3});
	m_regex_float.SetTransition({0, 1}, trFunc_isNumeric);
	m_regex_float.SetTransition({0, 2}, trFunc_isMinus);
	m_regex_float.SetTransition({0, 3}, trFunc_isDot);
	m_regex_float.SetTransition({1, 1}, trFunc_isNumeric);
	m_regex_float.SetTransition({1, 3}, trFunc_isDot);
	m_regex_float.SetTransition({2, 2}, trFunc_isNumeric);
	m_regex_float.SetTransition({2, 3}, trFunc_isDot);
	m_regex_float.SetTransition({3, 3}, trFunc_isNumeric);
		
	m_regex_string = DFA(6, 0, {5});
	m_regex_string.SetTransition({0, 1}, trFunc_isDoubleQuote);
	m_regex_string.SetTransition({1, 1}, trFunc_notBackslashAndDoubleQuote);
	m_regex_string.SetTransition({1, 2}, trFunc_isBackslash);
	m_regex_string.SetTransition({2, 1}, trFunc_isEscapeDouble);
	m_regex_string.SetTransition({1, 5}, trFunc_isDoubleQuote);
	m_regex_string.SetTransition({0, 3}, trFunc_isSingleQuote);
	m_regex_string.SetTransition({3, 3}, trFunc_notBackslashAndSingleQuote);
	m_regex_string.SetTransition({3, 4}, trFunc_isBackslash);
	m_regex_string.SetTransition({4, 3}, trFunc_isEscapeSingle);
	m_regex_string.SetTransition({3, 5}, trFunc_isSingleQuote);
}