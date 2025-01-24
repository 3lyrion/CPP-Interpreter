#pragma once

#include <Tree.h>

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

	struct Variable
	{
		enum class Type : char
		{
			Any = -1,
			Bool = 0,
			Float,
			Int,
			String
		};

		optional<string> id{};
		Type             type{};
		Value            value{};

		inline Variable& operator = (Variable const& var)
		{
			type  = var.type;
			value = var.value;

			return *this;
		}
	};

	using VType       = Variable::Type;
	using TkValueType = Shell::Token::Value::Type;
	using Block       = list<Variable>; 

	Tree<Token>* tree{};

	list<Block> m_blocks;

private:
	void declaration();

	void expression(Variable& target);

	void expressionStatement();

	void iterationStatement();

	void printStatement();

	void selectionStatement();

	Block& openBlock();

	void closeBlock();

private:
	Variable& search(string const& id);

	void initialize(Variable& var);
	void initialize(Variable& var, string const& value);
	
	Variable& declare(string const& id, VType type);

	// Temporary variable
	Variable& declare();

	// Temporary variable
	Variable& declare(VType type);

	// Temporary variable
	Variable& declare(VType type, string const& value);

	// Temporary variable
	Variable& declare(VType type, Value const& value);

	void unaryOp(char op, Variable& target);

	void arithmOp(char op, Variable& target, Variable& lvar, Variable const& rvar);
	void arithmOp(char op, Variable& target, Variable& lvar, string const& rvalue);

	void logicOp(string const& op, Variable& target, Variable& lvar, Variable const& rvar);
	void logicOp(string const& op, Variable& target, Variable& lvar, string const& rvalue);

	VType toVIType(TkValueType type) const;
	VType toVIType(char type)        const;

	ValuePtr toValue(Token::Value const& tokenValue) const;

	using Expression = list<Token::Value const*>;
	Expression toPostfix(Expression const& infix) const;

	string toString(VType type) const;

	bool stob(string const& value) const;

	string getValue(Variable const& var) const;

	void throwConversionError(Variable const& lvar, string const& rvalue, string const& op, exception const& e) const;

	void throwIncompatibilityError(Variable const& lvar, Variable const& rvar, string const& op) const;
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