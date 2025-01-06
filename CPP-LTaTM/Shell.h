#pragma once

#include "Tree.h"

class Shell
{


public:
	class Token
	{
	public:
		struct Hash
		{
			inline size_t operator () (Token const& token) const
			{
				return visit(

					[](auto&& arg)
					{
						return hash<decay_t<decltype(arg)>>()(arg);
					},

					token.m_variant
				);

			}
		};

	public:
		enum class Type
		{
			Declaration = 0,
			Expression,
			ExpressionStatement,
			IterationStatement,
			PrintStatement,
			SelectionStatement
		};

		Token(Type type)
		{
			m_variant.emplace<Type>(type);
		}

		Token(const char value[])
		{
			m_variant.emplace<string>(value);
		}

		Token(string const& value)
		{
			m_variant.emplace<string>(value);
		}

		inline Type const* getType() const
		{
			return get_if<Type>(&m_variant);
		}

		inline string const* getValue() const
		{
			return get_if<string>(&m_variant);
		}

		inline bool operator == (Token const& token) const
		{
			return m_variant == token.m_variant;
		}

		inline bool operator != (Token const& token) const
		{
			return m_variant != token.m_variant;
		}

		friend ostream& operator << (ostream&, Token const&);

	private:
		variant<Type, string> m_variant;
	};

	enum class Type : char
	{
		Bool,
		Float,
		Int,
		String
	};

	using Value = variant<bool, int, float, string>;
	struct ValueInfo
	{
		Type  type;
		Value value;
	};

	struct Block
	{
		vector<string>    ids;
		vector<ValueInfo> infos;

		ValueInfo* target = nullptr;

		bool   repeat = false;
		size_t depth  = 0;
	};

	Tree<Token>* tree = nullptr;

	stack<Block> m_stack;
	bool         m_repeat    = false;
	bool         m_condition = false;

	void dive()
	{
		tree->down();

		auto& top = m_stack.top();
		if (top.repeat)
			top.depth++;
	}

	void interpet(Tree<Token>& theTree)
	{
		tree = &theTree;

		bool repeat = false;

		tree->begin();
		tree->down();

		//tree->down();
		//			prepare(tree->get().value);
		//			tree->up();

		while (true)
		{
			auto& token = tree->get();

			if (auto type = token.getType())
			{
				switch (*type)
				{
				case Token::Type::Declaration:
					declaration();
				break;

				case Token::Type::Expression:
					expression();
				break;

				case Token::Type::ExpressionStatement:
					expressionStatement();
				break;

				case Token::Type::IterationStatement:
					iterationStatement();
				break;

				case Token::Type::PrintStatement:
					printStatement();
				break;

				case Token::Type::SelectionStatement:
					selectionStatement();
				break;

				default:
					break;
				}
			}

			else
			{
				auto& value = *token.getValue();

				if (value[0] == '{')
				{
					if (m_condition)
					{
						m_condition = false;

						if (!get<bool>(closeBlock().value))
							tree->up();

						else
							openBlock();
					}

					else
						openBlock();
				}

				else if (value[0] == '}')
				{
					closeBlock();
				}
			}

		foot:

			if (!tree->next())
			{
				auto& top = m_stack.top();
				// Если доступен возврат к условию
				if (top.depth != 0ull)
				{
					do
					{
						tree->up();
						top.depth--;
					}
					while (top.depth != 0ull);

					m_stack.pop();

					while (true)
					{
						tree->prev();
						auto& token = tree->get();

						if (auto type = token.getType())
						{
							if (*type == Token::Type::IterationStatement)
								break;
						}
					}
					tree->next();
				}
			}

			else 
			{
				bool end = false;

				tree->up();
				while (!tree->next())
				{
					if (auto value = tree->get().getValue())
					{
						if (*value == "program")
						{
							end = true;
							break;
						}
					}

					tree->up();
				}

				if (end)
					break;
			}

		}
	}

	void prepare(string const& id)
	{
	//	auto& [ids, infos, target] = m_stack.top();

		//auto entry = find(ids.cbegin(), ids.cend(), id);
		//if (entry == ids.end())
		//	throw runtime_error("");

		//target = &infos[distance(ids.cbegin(), entry)];
	}

	void declaration()
	{
		tree->down();
		auto& type = tree->get();

		tree->next();
		tree->down();
		auto& id = tree->get();
		tree->up();

		//declare(id.value, getType(type.value));

		if (tree->next()) // =
		{
			tree->next();
			auto& value = tree->get();

		//	assign(id.value, getType(value.type), value.value);
		}

		tree->up();
	}

	void expression()
	{
		m_stack.emplace();

		tree->down();

		// calculations...

		tree->up();

		auto result = m_stack.top().infos.back();
		m_stack.pop();

		auto target = m_stack.top().target;
		if (target)
		{
			if (target->type == result.type)
				target->value = result.value;
			
			else
				throw runtime_error("");
		}
	}

	void expressionStatement()
	{
		//if (token.value == "if")
		//			{
		//				openBlock();
		//				m_condition = true;
		//			}

		//			else if (token.value == "else")
		//			{
		//				openElse();
		//			}

		//			else if (token.value == "while")
		//			{
		//				m_repeat = true;
		//			}
	}

	void iterationStatement()
	{

	}

	void printStatement()
	{

	}

	void selectionStatement()
	{

	}

	void openBlock()
	{
		m_stack.emplace();
	}

	ValueInfo closeBlock()
	{
		auto value = m_stack.top().infos.back();

		m_stack.pop();

		return value;
	}
	/*
	void declare(string const& id, Type type)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry != ids.end())
			throw runtime_error("");

		ids.emplace_back(id);
		auto& info = infos.emplace_back();
		info.type  = type;
	}

	// Temporary variable
	void declare(Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		ids.emplace_back();
		auto& info = infos.emplace_back();
		info.type  = type;
		initialize(info, value);
	}

	void assign(string const& id, Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry == ids.end())
			throw runtime_error("");

		auto& info = infos[distance(ids.cbegin(), entry)];

		if (info.type != type)
			throw runtime_error("");

		initialize(info, value);
	}

	void assign(string const& id_lhs, string const& id_rhs)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto entry_lhs = find(ids.cbegin(), ids.cend(), id_lhs);
		if (entry_lhs == ids.end())
			throw runtime_error("");

		auto entry_rhs = find(ids.cbegin(), ids.cend(), id_rhs);
		if (entry_rhs == ids.end())
			throw runtime_error("");

		auto& info_lhs = infos[distance(ids.cbegin(), entry_lhs)];
		auto& info_rhs = infos[distance(ids.cbegin(), entry_rhs)];

		if (info_lhs.type != info_rhs.type)
			throw runtime_error("");

		info_lhs.value = info_rhs.value;
	}

	void execMathOp(char op, string const& id, Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry == ids.end())
			throw runtime_error("");

		auto& info = infos[distance(ids.cbegin(), entry)];
		if (type != info.type)
			throw runtime_error("");

		declCopy(info);

		mathOp(op, infos.back(), value);
	}

	// Temporary variable
	void execMathOp(char op, Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto& info = infos.back();
		if (type != info.type)
			throw runtime_error("");

		mathOp(op, info, value);
	}

	void execLogicOp(string const& op, string const& id, Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry == ids.end())
			throw runtime_error("");

		auto& info = infos[distance(ids.cbegin(), entry)];
		if (type != info.type)
			throw runtime_error("");

		declCopy(info);

		logicOp(op, infos.back(), value);
	}

	// Temporary variable
	void execLogicOp(string const& op, Type type, string const& value)
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		auto& info = infos.back();
		if (type != info.type)
			throw runtime_error("");

		logicOp(op, info, value);
	}

	void openElse()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			skip = false;
	}
	*/

	bool stob(string const& value) const
	{
		if (value.front() == 't' || value.front() == '1')
			return true;

		return false;
	}

	bool isInitialized(Value const& value) const
	{
		return value.index() != variant_npos;
	}

	void initialize(ValueInfo& info, string const& value)
	{
		switch (info.type)
		{
		case Type::Bool:
			info.value.emplace<bool>(stob(value));
		break;

		case Type::Float:
			info.value.emplace<float>(stof(value));
		break;

		case Type::Int:
			info.value.emplace<int>(stoi(value));
		break;

		case Type::String:
			info.value.emplace<string>(value);
		break;

		default:
			break;
		}
	}

	/*void declUninit(Type type)
	{
		auto& [skip, ids, infos, _] = m_stack.top();

		ids.emplace_back();
		auto& _info = infos.emplace_back();
		_info.type  = type;
	}

	void declCopy(ValueInfo& info)
	{
		auto& [skip, ids, infos, _] = m_stack.top();

		ids.emplace_back();
		auto& _info = infos.emplace_back();
		_info.type  = info.type;
		_info.value = info.value;
	}*/

	void mathOp(char op, ValueInfo& info, string const& value)
	{
		switch (op)
		{
		case '^':
		{
			switch (info.type)
			{
			case Type::Float:
			{
				auto& _value = get<float>(info.value);
				_value = pow(_value, stof(value));
			}
			break;

			case Type::Int:
			{
				auto& _value = get<int>(info.value);
				_value = (int)pow(_value, stoi(value));
			}
			break;

			case Type::String:
			break;

			default:
				break;
			}
		}
		break;

		case '*':
		{
			switch (info.type)
			{
			case Type::Float:
				get<float>(info.value) *= stof(value);
			break;

			case Type::Int:
				get<int>(info.value) *= stoi(value);
			break;

			case Type::String:
			break;

			default:
				break;
			}
		}
		break;

		case '/':
		{
			switch (info.type)
			{
			case Type::Float:
				get<float>(info.value) /= stof(value);
			break;

			case Type::Int:
				get<int>(info.value) /= stoi(value);
			break;

			case Type::String:
			break;

			default:
				break;
			}
		}
		break;

		case '%':
		{
			switch (info.type)
			{
			case Type::Float:
			break;

			case Type::Int:
				get<int>(info.value) %= stoi(value);
			break;

			case Type::String:
			break;

			default:
				break;
			}
		}
		break;

		case '+':
		{
			switch (info.type)
			{
			case Type::Float:
				get<float>(info.value) += stof(value);
			break;

			case Type::Int:
				get<int>(info.value) += stoi(value);
			break;

			case Type::String:
				get<string>(info.value) += value;
			break;

			default:
				break;
			}
		}
		break;

		case '-':
		{
			switch (info.type)
			{
			case Type::Float:
				get<float>(info.value) -= stof(value);
			break;

			case Type::Int:
				get<int>(info.value) -= stoi(value);
			break;

			case Type::String:
			break;

			default:
				break;
			}
		}
		break;
		}
	}

	void logicOp(string const& op, ValueInfo const& info, string const& value)
	{
	//	declUninit(Type::Bool);
		auto& temp_value = get<bool>(m_stack.top().infos.back().value);

		auto len = op.size();

		switch (op[0])
		{
		case '<':
		{
			if (len > 1ull)
				switch (info.type)
				{
				case Type::Float:
					temp_value = get<float>(info.value) <= stof(value);
				break;

				case Type::Int:
					temp_value = get<int>(info.value) <= stoi(value);
				break;

				case Type::String:
				break;
					
				default:
					break;
				}

			else
				switch (info.type)
				{
				case Type::Float:
					temp_value = get<float>(info.value) < stof(value);
				break;

				case Type::Int:
					temp_value = get<int>(info.value) < stoi(value);
				break;

				case Type::String:
				break;
					
				default:
					break;
				}
		}
		break;

		case '>':
		{
			if (len > 1ull)
				switch (info.type)
				{
				case Type::Float:
					temp_value = get<float>(info.value) >= stof(value);
				break;

				case Type::Int:
					temp_value = get<int>(info.value) >= stoi(value);
				break;

				case Type::String:
				break;
					
				default:
					break;
				}

			else
				switch (info.type)
				{
				case Type::Float:
					temp_value = get<float>(info.value) > stof(value);
				break;

				case Type::Int:
					temp_value = get<int>(info.value) > stoi(value);
				break;

				case Type::String:
				break;
					
				default:
					break;
				}
		}
		break;

		case '=':
		{
			switch (info.type)
			{
			case Type::Bool:
				temp_value = get<bool>(info.value) == stob(value);
			break;

			case Type::Float:
				temp_value = get<float>(info.value) == stof(value);
			break;

			case Type::Int:
				temp_value = get<int>(info.value) == stoi(value);
			break;

			case Type::String:
				temp_value = get<string>(info.value) == value;
			break;
					
			default:
				break;
			}
		}
		break;

		case '!':
		{
			switch (info.type)
			{
			case Type::Bool:
				temp_value = get<bool>(info.value) != stob(value);
			break;

			case Type::Float:
				temp_value = get<float>(info.value) != stof(value);
			break;

			case Type::Int:
				temp_value = get<int>(info.value) != stoi(value);
			break;

			case Type::String:
				temp_value = get<string>(info.value) != value;
			break;
					
			default:
				break;
			}
		}
		break;

		case '&':
		{
			switch (info.type)
			{
			case Type::Bool:
				temp_value = get<bool>(info.value) && stob(value);
			break;
					
			default:
				break;
			}
		}
		break;

		case '|':
		{
			switch (info.type)
			{
			case Type::Bool:
				temp_value = get<bool>(info.value) || stob(value);
			break;
					
			default:
				break;
			}
		}
		break;
		}
	}
};

inline ostream& operator << (ostream& os, Shell::Token const& token)
{
	if (auto type = token.getType())
	{
		switch (*type)
		{
		case Shell::Token::Type::Declaration:
			os << "declaration";
		break;

		case Shell::Token::Type::Expression:
			os << "expression";
		break;

		case Shell::Token::Type::ExpressionStatement:
			os << "expression_statement";
		break;

		case Shell::Token::Type::IterationStatement:
			os << "iteration_statement";
		break;

		case Shell::Token::Type::PrintStatement:
			os << "print_statement";
		break;

		case Shell::Token::Type::SelectionStatement:
			os << "selection_statement";
		break;

		default:
			break;
		}
	}

	else
		os << *token.getValue();

	return os;
}