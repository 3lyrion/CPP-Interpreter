#pragma once

#include "Context.h"
#include "Tree.h"

class Viewer
{
	unordered_set<char> m_mathOperators;

	inline Context::Type getType(string const& type) const
	{
		auto context_type = Context::Type::String;
		if      (type == "bool") context_type = Context::Type::Bool;
		else if (type == "float") context_type = Context::Type::Float;
		else if (type == "int")  context_type = Context::Type::Int;

		return context_type;
	}

	inline Context::Type getType(TokenType type) const
	{
		auto context_type = Context::Type::String;
		if      (type == TokenType::Keyword) context_type = Context::Type::Bool;
		else if (type == TokenType::FloatLiteral) context_type = Context::Type::Float;
		else if (type == TokenType::IntLiteral)  context_type = Context::Type::Int;

		return context_type;
	}

public:
	Viewer() = default;

	void interpet(Tree<Token>& tree)
	{
		tree.begin();
		tree.down();

		do
		{
			auto& token = tree.get();

			switch (token.type)
			{
			case TokenType::Separator:
			{
				if (token.value == "declaration")
				{
					tree.down();
					auto& type = tree.get();

					tree.next();
					tree.down();
					auto& id = tree.get();
					tree.up();

					m_context.declare(id.value, getType(type.value));

					if (tree.next()) // =
					{
						tree.next();
						auto& value = tree.get();

						m_context.assign(id.value, getType(value.type), value.value);
					}

					tree.up();

				}

				else if (token.value == "{")
				{
					if (m_conditionOpened)
					{
						m_context.closeCondition();
						m_conditionOpened = false;
					}

					m_context.openBlock();
				}

				else if (token.value == "}")
				{
					m_context.closeBlock();
				}
			}
			break;

			case TokenType::Id:
			{
				if (token.value == "declaration")
				{
					tree.down();
					auto& type = tree.get();

					tree.next();
					tree.down();
					auto& id = tree.get();
					tree.up();

					m_context.declare(id.value, getType(type.value));

					if (tree.next()) // =
					{
						tree.next();
						auto& value = tree.get();

						m_context.assign(id.value, getType(value.type), value.value);
					}

					tree.up();

				}
			}
			break;

			case TokenType::Keyword:
			{
				if (token.value == "if")
				{
					m_context.openCondition();
					m_conditionOpened = true;
				}

				else if (token.value == "else")
				{
					m_context.openElse();
				}
			}
			break;

			case TokenType::Operator:
			{
				if (token.value.size() == 1ull)
				{
				///	if (m_mathOperators.contains(token.value[0]))
				//		m_context.execMathOp
				}
			}
			break;

			default:
				break;
			}

		}
		while (tree.next());
	}

private:
	Context m_context;

	bool m_conditionOpened = false;
};