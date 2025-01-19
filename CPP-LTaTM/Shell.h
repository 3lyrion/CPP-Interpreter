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
	Shell() = default;

	void interpet(Tree<Token>& theTree);

private:
	using Value    = variant<bool, int, float, string>; // 48 bytes ()()()()
	using ValuePtr = unique_ptr<Value>;

	struct ValueInfo
	{
		enum class Type : char
		{
			Any = -1,
			Bool = 0,
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

		bool   repeat = false;
		size_t depth  = 0;
	};

	Tree<Token>* tree = nullptr;

	list<Block> m_blocks;

private:
	void dive();

	ValueInfo& search(string const& id);

//	void prepare(string const& id);

	void declaration();

	void expression(ValueInfo& target);

	void expressionStatement();

	void iterationStatement();

	void printStatement();

	void selectionStatement();

	Block& openBlock();

	void closeBlock();
	
	ValueInfo& declare(string const& id, VIType type);

	// Temporary variable
	ValueInfo& declare();

	// Temporary variable
	ValueInfo& declare(VIType type, string const& value = "");

	// Temporary variable
	ValueInfo& declare(VIType type, Value const& value);

	void assign(string const& id, VIType type, string const& value);

//	void assign(string const& id_lhs, string const& id_rhs);
	
	bool stob(string const& value) const;

	VIType toVIType(Token::Value::Type type) const;
	VIType toVIType(char type)               const;

	bool isInitialized(Value const& value) const;

	void initialize(ValueInfo& info);
	void initialize(ValueInfo& info, string const& value);

	ValuePtr arithmOp(char op, VIType type, Value const& lvalue, Value const& rvalue);
	ValuePtr arithmOp(char op, VIType type, string const& lvalue, string const& rvalue);

	void arithmOp(char op, ValueInfo& target, ValueInfo& linfo, Value const& rvalue);
	void arithmOp(char op, ValueInfo& target, ValueInfo& linfo, string const& rvalue);

	void logicOp(string const& op, ValueInfo& target, ValueInfo& linfo, Value const& rvalue);
	void logicOp(string const& op, ValueInfo& target, ValueInfo& linfo, string const& rvalue);
	//void logicOp(string const& op, VIType type, Value const& lvalue, Value const& rvalue);
	//void logicOp(string const& op, VIType type, string const& lvalue, string const& rvalue);

	ValuePtr toValue(Token::Value const& tokenValue) const;

	using Expression = list<Token::Value const*>;
	Expression toPostfix(Expression const& infix) const;
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
		os << token.getValue()->text;

	return os;
}