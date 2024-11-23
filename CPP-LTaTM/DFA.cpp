#include "DFA.hpp"

DFA::DFA(size_t size, State initialState, initializer_list<State> finalStates)
{
	if (size < 2)
		throw exception("DFA must contain more than two states");

	m_size = size;

	if (!IsLegalState(initialState))
		throw exception("Passed initial state is illegal");

	m_initialState = initialState;
	m_currentState = initialState;

	if (finalStates.size() < 1)
		throw exception("DFA must have at least one final state");

	for (State s : finalStates)
		m_finalStates.push_back(s);

	for (size_t i = 0; i < size; ++i) {
		
		auto& row = m_transitions.emplace_back();

		for (size_t j = 0; j < size; ++j)
			row.push_back(NoTransition);
	}
	
}

DFA& DFA::operator= (const DFA& other)
{
	
	m_transitions = other.m_transitions;
	m_size = other.m_size;
	m_currentState = other.m_currentState;
	m_initialState = other.m_initialState;
	m_finalStates = other.m_finalStates;

	return *this;
	
}

DFA& DFA::operator= (DFA&& other) noexcept {
		
	m_transitions = other.m_transitions;
	m_size = other.m_size;
	m_currentState = other.m_currentState;
	m_initialState = other.m_initialState;
	m_finalStates = other.m_finalStates;

	return *this;
		
}

bool DFA::IsLegalState(State s) {
	
	if (s >= 0 && s < m_size)
		return true;

	return false;

}

bool DFA::IsLegalTransition(Transition t) {
	
	return (IsLegalState(t.first) && IsLegalState(t.second));
	
}

bool DFA::IsInFinalState() {
	
	for (State s : m_finalStates)
		if (m_currentState == s)
			return true;

	return false;

}

void DFA::SetTransition(Transition t, TransitionPred f) {
	
	if (!IsLegalTransition(t))
		return;

	m_transitions[t.first][t.second] = f;
	
}

bool DFA::Move(char input) {
		
	bool isAccepted = false;
	size_t nextState = 0;

	for (TransitionPred f : m_transitions[m_currentState]) {
		
		if (f(input)) {
			
			isAccepted = true;
			m_currentState = nextState;
			break;
			
		}

		++nextState;
		
	}

	return isAccepted;
	
}

bool DFA::Process(const string& input) {
	
	if (input.size() == 0)
		return false;

	for (char c : input) {
		
		if (!Move(c)) {

			if (m_currentState != m_initialState)
			{
				m_currentState = m_initialState;

				throw input;
			}

			return false;

		}
	}

	m_currentState = m_initialState;
	return true;
	
}

void DFA::Reset() {
	
	m_currentState = m_initialState;
	
}
