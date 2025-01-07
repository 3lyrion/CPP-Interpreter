#pragma once

#include "Tree.h"

class Shell
{
public:
	class Token
	{
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

		struct Value
		{
			enum class Type : int
			{
				Other = 0,
				Id,
				Operator,
				BoolLiteral,
				FloatLiteral,
				IntLiteral,
				StringLiteral
			}
			type;

			string text;

			Value(Type theType, string const& theText) :
				type(theType),
				text(theText) { }

			inline bool operator == (Value const& value) const
			{
				return type == value.type && text == value.text;
			}

			inline bool operator != (Value const& value) const
			{
				return type != value.type || text != value.text;
			}

			struct Hash
			{
				inline size_t operator () (Value const& value) const
				{
					return hash<string>()(value.text) ^ (hash<int>()(int(value.type)) << 1);
				}
			};
		};

		struct Hash
		{
			inline size_t operator () (Token const& token) const
			{
				return visit(

					[](auto&& arg)
					{
						using T = decay_t<decltype(arg)>;

						if constexpr (is_same<T, Token::Value>())
							return Value::Hash()(arg);

						else
							return hash<int>()(int(arg));
					},

					token.m_variant
				);
			}
		};

		Token(Type type)
		{
			m_variant.emplace<Type>(type);
		}

		Token(const char valueText[], Value::Type valueType = Value::Type::Other)
		{
			m_variant.emplace<Value>(valueType, valueText);
		}

		Token(string const& valueText, Value::Type valueType = Value::Type::Other)
		{
			m_variant.emplace<Value>(valueType, valueText);
		}

		inline Type const* getType() const
		{
			return get_if<Type>(&m_variant);
		}

		inline Value const* getValue() const
		{
			return get_if<Value>(&m_variant);
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
		variant<Type, Value> m_variant;
	};

public:
	void interpet(Tree<Token>& theTree)
	{
		tree = &theTree;

		bool repeat = false;

		tree->begin();
		tree->down();

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
				auto& text = token.getValue()->text;

				if (text[0] == '{')
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

				else if (text[0] == '}')
				{
					closeBlock();
				}
			}

			if (!tree->next())
			{
				auto& back = m_blocks.back();
				// Если доступен возврат к условию
				if (back.depth != 0ull)
				{
					do
					{
						tree->up();
						back.depth--;
					}
					while (back.depth != 0ull);

					m_blocks.pop_back();

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
						if (value->text == "program")
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

private:
	using Value = variant<bool, int, float, string>;
	struct ValueInfo
	{
		enum class Type : char
		{
			Bool,
			Float,
			Int,
			String
		};

		Type  type;
		Value value;
	};

	using VIType = ValueInfo::Type;

	struct Block
	{
		vector<string>    ids;
		vector<ValueInfo> infos;

		ValueInfo* target = nullptr;

		bool   repeat = false;
		size_t depth  = 0;
	};

	Tree<Token>* tree = nullptr;

	list<Block> m_blocks;
	bool        m_repeat    = false;
	bool        m_condition = false;

	void dive()
	{
		tree->down();

		auto& back = m_blocks.back();
		if (back.repeat)
			back.depth++;
	}

	ValueInfo* findIf(string const& id)
	{
		for (auto& block : m_blocks)
		{
			auto& ids = block.ids;

			auto entry = find(ids.cbegin(), ids.cend(), id);
			if (entry != ids.cend())
				return &block.infos[distance(ids.cbegin(), entry)];
		}

		return nullptr;
	}

	void prepare(string const& id)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry == ids.end())
			throw runtime_error("");

		back.target = &infos[distance(ids.cbegin(), entry)];
	}

	void declaration()
	{
		tree->down();
		auto type = toVIType(tree->get().getValue()->type);

		tree->next();
		tree->down();
		auto& id = tree->get().getValue()->text;
		tree->up();

		declare(id, type);

		if (tree->next()) // =
		{
			tree->next();

			// expression ???????

			auto& value = tree->get().getValue()->text;

			assign(id, type, value);
		}

		tree->up();
	}

	void expression()
	{
		m_blocks.emplace_back();

		dive();

		// Reverse Polish Notation
		vector<Token::Value const*> rpn;

		do
		{
			auto& token = tree->get();

			if (auto type = token.getType())
			{
				if (*type == Token::Type::Expression)
					expression();
			}

			else
				rpn.push_back(token.getValue());

		} while (tree->next());

		// TODO
		//toRPN(rpn); 

		for (size_t i = 0; i < rpn.size() - 1ull; i += 2ull)
		{
			auto& lhs = rpn[i];
			if (lhs->type != Token::Value::Type::Operator)
			{
				// Создание копии существующей переменной
				if (lhs->type == Token::Value::Type::Id)
				{
					auto& vi = *findIf(lhs->text);
					declare(vi.type, vi.value);
				}

				// Создание переменной из литерала
				else
					declare(toVIType(lhs->type), lhs->text);

				auto& rhs = rpn[i + 1ull];

				// Searching for operator...
				for (size_t j = i; i < rpn.size(); i++)
				{
					if (rpn[j]->type == Token::Value::Type::Operator)
					{
						auto& _text = rpn[j]->text;

						if (_text[0] == '<' || _text[0] == '>' || _text.size() == 2ull)
							logicOp(_text, m_blocks.back().infos.back(), rhs->text);

						else
							arithmOp(_text[0], m_blocks.back().infos.back(), rhs->text);

						break;
					}
				}
			}
		}

		tree->up();

		auto result = m_blocks.back().infos.front();
		m_blocks.pop_back();

		auto target = m_blocks.back().target;
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
		
	}

	void iterationStatement()
	{

	}

	void printStatement()
	{

	}

	void selectionStatement()
	{
		dive();
		auto& token = tree->get();

		if (auto value = token.getValue())
		{
			if (value->type == Token::Value::Type::Other)
			{
				auto& text = value->text;
				if (text == "if")
				{
					openBlock();
					m_condition = true;
				}

				else if (text == "else")
				{
			//		openElse();
				}

				else if (text == "while")
				{
					m_blocks.back().repeat = true;
				}
			}
		}
	}

	void openBlock()
	{
		m_blocks.emplace_back();
	}

	ValueInfo closeBlock()
	{
		auto value = m_blocks.back().infos.back();

		m_blocks.pop_back();

		return value;
	}
	
	void declare(string const& id, VIType type)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry != ids.end())
			throw runtime_error("");

		ids.emplace_back(id);
		auto& info = infos.emplace_back();
		info.type  = type;
	}

	// Temporary variable
	void declare(VIType type, string const& value)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		ids.emplace_back();
		auto& info = infos.emplace_back();
		info.type  = type;
		initialize(info, value);
	}

	// Temporary variable
	void declare(VIType type, Value const& value)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		ids.emplace_back();
		auto& info = infos.emplace_back();
		info.type  = type;
		info.value = value;
	}

	void assign(string const& id, VIType type, string const& value)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

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
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

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

	//void execMathOp(char op, string const& id, VIType type, string const& value)
	//{
	//	auto& [skip, ids, infos, _] = m_blocks.back();
	//	if (skip)
	//		return;

	//	auto entry = find(ids.cbegin(), ids.cend(), id);
	//	if (entry == ids.end())
	//		throw runtime_error("");

	//	auto& info = infos[distance(ids.cbegin(), entry)];
	//	if (type != info.type)
	//		throw runtime_error("");

	//	declCopy(info);

	//	mathOp(op, infos.back(), value);
	//}

	//// Temporary variable
	//void execMathOp(char op, VIType type, string const& value)
	//{
	//	auto& [skip, ids, infos, _] = m_blocks.back();
	//	if (skip)
	//		return;

	//	auto& info = infos.back();
	//	if (type != info.type)
	//		throw runtime_error("");

	//	mathOp(op, info, value);
	//}



	//void execLogicOp(string const& op, string const& id, VIType type, string const& value)
	//{
	//	auto& [skip, ids, infos, _] = m_blocks.back();
	//	if (skip)
	//		return;

	//	auto entry = find(ids.cbegin(), ids.cend(), id);
	//	if (entry == ids.end())
	//		throw runtime_error("");

	//	auto& info = infos[distance(ids.cbegin(), entry)];
	//	if (type != info.type)
	//		throw runtime_error("");

	//	declCopy(info);

	//	logicOp(op, infos.back(), value);
	//}

	//// Temporary variable
	//void execLogicOp(string const& op, VIType type, string const& value)
	//{
	//	auto& [skip, ids, infos, _] = m_blocks.back();
	//	if (skip)
	//		return;

	//	auto& info = infos.back();
	//	if (type != info.type)
	//		throw runtime_error("");

	//	logicOp(op, info, value);
	//}

	//void openElse()
	//{
	//	auto& [skip, ids, infos, _] = m_blocks.back();
	//	if (skip)
	//		skip = false;
	//}
	

	bool stob(string const& value) const
	{
		if (value.front() == 't' || value.front() == '1')
			return true;

		return false;
	}

	VIType toVIType(Token::Value::Type type) const
	{
		switch (type)
		{
		case Token::Value::Type::BoolLiteral:
			return VIType::Bool;

		case Token::Value::Type::FloatLiteral:
			return VIType::Float;

		case Token::Value::Type::IntLiteral:
			return VIType::Int;

		case Token::Value::Type::StringLiteral:
			return VIType::String;

		default:
			throw runtime_error("");
		}
	}

	bool isInitialized(Value const& value) const
	{
		return value.index() != variant_npos;
	}

	void initialize(ValueInfo& info, string const& value)
	{
		switch (info.type)
		{
		case VIType::Bool:
			info.value.emplace<bool>(stob(value));
		break;

		case VIType::Float:
			info.value.emplace<float>(stof(value));
		break;

		case VIType::Int:
			info.value.emplace<int>(stoi(value));
		break;

		case VIType::String:
			info.value.emplace<string>(value);
		break;

		default:
			break;
		}
	}

	void declUninit(VIType type)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		ids.emplace_back();
		auto& _info = infos.emplace_back();
		_info.type  = type;
	}

	void declCopy(ValueInfo& info)
	{
		auto& back   = m_blocks.back();
		auto& ids   = back.ids;
		auto& infos = back.infos;

		ids.emplace_back();
		auto& _info = infos.emplace_back();
		_info.type  = info.type;
		_info.value = info.value;
	}

	void arithmOp(char op, ValueInfo& info, string const& value)
	{
		switch (op)
		{
		case '^':
		{
			switch (info.type)
			{
			case VIType::Float:
			{
				auto& _value = get<float>(info.value);
				_value = pow(_value, stof(value));
			}
			break;

			case VIType::Int:
			{
				auto& _value = get<int>(info.value);
				_value = (int)pow(_value, stoi(value));
			}
			break;

			case VIType::String:
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
			case VIType::Float:
				get<float>(info.value) *= stof(value);
			break;

			case VIType::Int:
				get<int>(info.value) *= stoi(value);
			break;

			case VIType::String:
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
			case VIType::Float:
				get<float>(info.value) /= stof(value);
			break;

			case VIType::Int:
				get<int>(info.value) /= stoi(value);
			break;

			case VIType::String:
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
			case VIType::Float:
			break;

			case VIType::Int:
				get<int>(info.value) %= stoi(value);
			break;

			case VIType::String:
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
			case VIType::Float:
				get<float>(info.value) += stof(value);
			break;

			case VIType::Int:
				get<int>(info.value) += stoi(value);
			break;

			case VIType::String:
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
			case VIType::Float:
				get<float>(info.value) -= stof(value);
			break;

			case VIType::Int:
				get<int>(info.value) -= stoi(value);
			break;

			case VIType::String:
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
	//	declUninit(VIType::Bool);
		auto& temp_value = get<bool>(m_blocks.back().infos.back().value);

		auto len = op.size();

		switch (op[0])
		{
		case '<':
		{
			if (len > 1ull)
				switch (info.type)
				{
				case VIType::Float:
					temp_value = get<float>(info.value) <= stof(value);
				break;

				case VIType::Int:
					temp_value = get<int>(info.value) <= stoi(value);
				break;

				case VIType::String:
				break;
					
				default:
					break;
				}

			else
				switch (info.type)
				{
				case VIType::Float:
					temp_value = get<float>(info.value) < stof(value);
				break;

				case VIType::Int:
					temp_value = get<int>(info.value) < stoi(value);
				break;

				case VIType::String:
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
				case VIType::Float:
					temp_value = get<float>(info.value) >= stof(value);
				break;

				case VIType::Int:
					temp_value = get<int>(info.value) >= stoi(value);
				break;

				case VIType::String:
				break;
					
				default:
					break;
				}

			else
				switch (info.type)
				{
				case VIType::Float:
					temp_value = get<float>(info.value) > stof(value);
				break;

				case VIType::Int:
					temp_value = get<int>(info.value) > stoi(value);
				break;

				case VIType::String:
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
			case VIType::Bool:
				temp_value = get<bool>(info.value) == stob(value);
			break;

			case VIType::Float:
				temp_value = get<float>(info.value) == stof(value);
			break;

			case VIType::Int:
				temp_value = get<int>(info.value) == stoi(value);
			break;

			case VIType::String:
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
			case VIType::Bool:
				temp_value = get<bool>(info.value) != stob(value);
			break;

			case VIType::Float:
				temp_value = get<float>(info.value) != stof(value);
			break;

			case VIType::Int:
				temp_value = get<int>(info.value) != stoi(value);
			break;

			case VIType::String:
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
			case VIType::Bool:
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
			case VIType::Bool:
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

//struct Hash
//{
//	inline size_t operator () (Shell::Token const& token) const
//	{
//		/*return hash<decltype(token.m_variant)>()(token.m_variant);*/
//
//
//		return visit(
//
//			[](const auto& item)
//			{
//				return hash<decay_t<decltype(item)>>()(item);
//			},
//
//			token.m_variant
//		);
//
//	}
//
//	//inline size_t operator () (Shell::Token::Value const& value) const
//	//{
//	//	return hash<int>()(int(value.type)) ^ hash<string>()(value.text);
//
//	//}
//
//	//inline size_t operator () (variant<Shell::Token::Type, Shell::Token::Value> const& v) const
//	//{
//	//	/*return hash<decltype(token.m_variant)>()(token.m_variant);*/
//
//	//	hash<
//
//
//	//	return visit(
//
//	//		[](auto&& arg)
//	//		{
//	//			return hash<decay_t<decltype(arg)>>()(arg);
//	//		},
//
//	//		v
//	//	);
//
//	//}
//};

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
		os << token.getValue()->text;

	return os;
}