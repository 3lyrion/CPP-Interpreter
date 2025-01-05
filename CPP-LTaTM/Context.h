#pragma once

#include "Lexer.hpp"

class Context
{
public:
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
		bool              skip  = false;
		vector<string>    ids;
		vector<ValueInfo> infos;

		bool repeat = false;
	};

	void openBlock()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		m_stack.emplace();
	}

	void closeBlock()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		m_stack.pop();
	}

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

	void openCondition()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		m_stack.emplace();
	}

	void closeCondition()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			return;

		if (!get<bool>(m_stack.top().infos.back().value))
			skip = true;

		m_stack.pop();
	}

	void openElse()
	{
		auto& [skip, ids, infos, _] = m_stack.top();
		if (skip)
			skip = false;
	}



private:
	stack<Block> m_stack;

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

	void declUninit(Type type)
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
	}

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
		declUninit(Type::Bool);
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

