#pragma once

#include "Precompiled.h"


	class DFA {
	
	public:

		using State          = size_t;
		using Transition     = pair<State, State>;
		using TransitionPred = bool(*)(char);
		
		const TransitionPred NoTransition  = [](char) -> bool { return false; };
		const TransitionPred HasTransition = [](char) -> bool { return true; };

	public:
		
		DFA() = default;

		DFA(size_t size, State initialState, initializer_list<State> finalStates);

		DFA& operator= (const DFA& other);

		DFA& operator= (DFA&& other) noexcept;

	public:

		bool IsLegalState(State s);

		bool IsInFinalState();

		bool IsLegalTransition(Transition t);

	public:

		void SetTransition(Transition t, TransitionPred f);

		bool Move(char input);

		bool Process(const string& input);

		void Reset();

	private:
		
		vector<vector<TransitionPred>> m_transitions;

		size_t m_size;

		State              m_currentState;
		State              m_initialState;
		vector<State> m_finalStates;
	
	};