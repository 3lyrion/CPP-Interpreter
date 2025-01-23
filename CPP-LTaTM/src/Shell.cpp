#include "Shell.h"

void Shell::interpet(Tree<Token>& theTree)
{
	tree = &theTree;

	//tree->print();
	//return;

	openBlock();

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
				openBlock();

			else if (text[0] == '}')
				closeBlock();
		}

		if (!tree->next())
		{
			tree->up();

			if (auto type = tree->get().getType())
			{
				if (*type == Token::Type::IterationStatement)
					continue;
			}

			bool end = false;
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

	closeBlock();
}

Shell::Variable& Shell::search(string const& id)
{
	for (auto& block : m_blocks)
	{
		auto entry = find_if(block.begin(), block.end(),
			[&id](Variable const& var)
			{
				if (var.id == id)
					return true;

				return false;
			}
		);

		if (entry != block.end())
			return *entry;
	}

	throw runtime_error("");
}

void Shell::declaration()
{
	tree->down();

	auto type = toVIType(tree->get().getValue()->text[0]);

	tree->next(); // type

	auto& id = tree->get().getValue()->text;

	auto& var_id = declare(id, type);

	if (tree->next()) // id
	{
		tree->next(); // =

		expression(var_id);
	}

	tree->up();
}

void Shell::expression(Variable& target)
{
	tree->down();

	auto& vars = openBlock();

	// Reverse Polish Notation
	Expression rpn;

	auto depth = 0u;

	while (true)
	{
		if (auto type = tree->get().getType())
		{
			if (*type == Token::Type::Expression)
			{
				tree->down();
				depth++;
			}
		}

		rpn.push_back(tree->get().getValue());

		if (!tree->next())
		{
			if (depth > 0u)
			{
				tree->up();
				tree->next();
				depth--;
			}

			else
				break;
		}

	}

	auto decl = [this](Token::Value const& value) -> auto&
	{
		// Создание копии существующей переменной
		if (value.type == TkValueType::Id)
		{
			auto& vi = search(value.text);
			return declare(vi.type, vi.value);
		}

		// Создание переменной из литерала
		else
			return declare(toVIType(value.type), value.text);
	};

	if (rpn.size() == 1ull)
	{
		auto& value = *rpn.front();

		if (value.type == TkValueType::Id)
			target = search(value.text);

		else
		{
			target.type = toVIType(value.type);
			initialize(target, value.text);
		}
	}

	else
	{
		rpn = toPostfix(rpn);

		while (!rpn.empty())
		{
			auto value = rpn.front();
			rpn.pop_front();

			if (value->type != TkValueType::Operator)
				decl(*value);

			else
			{
				auto& text = value->text;

				if (text[0] == '~' || text[0] == '!')
					unaryOp(text[0], vars.back());

				else
				{
					auto rhs = move(vars.back()); vars.pop_back();
					auto lhs = move(vars.back()); vars.pop_back();

					auto& var = declare();

					if (text[0] == '<' || text[0] == '>' || text.size() == 2ull)
						logicOp(text, var, lhs, rhs.value);

					else
						arithmOp(text[0], var, lhs, rhs.value);
				}
			}

		}

		target = vars.back();
	}

	tree->up();
	closeBlock();
}

void Shell::expressionStatement()
{
	tree->down();

	auto& id = tree->get().getValue()->text;

	auto& var_id = search(id);

	tree->next(); // id

	tree->next(); // =

	expression(var_id);

	tree->up();
}

void Shell::iterationStatement()
{
	tree->down();

	tree->next(); // while
	tree->next(); // (

	auto& var = declare(VType::Bool);
	expression(var);

	tree->next(); // expression

	// фальш
	if (get<bool>(var.value) == false)
		tree->up(); 
}

void Shell::printStatement()
{
	tree->down();
	tree->next();

	auto& var = declare();
	expression(var);

	switch (var.type)
	{
	case VType::Bool:
		cout << get<bool>(var.value);
	break;

	case VType::Float:
		cout << get<float>(var.value);
	break;

	case VType::Int:
		cout << get<int>(var.value);
	break;

	case VType::String:
		cout << get<string>(var.value);
	break;
	}

	cout << '\n';

	tree->up();
}

void Shell::selectionStatement()
{
	tree->down();

	tree->next(); // if
	tree->next(); // (

	auto& var = declare(VType::Bool);
	expression(var);

	tree->next(); // expression
	tree->next(); // )

	// фальш
	if (get<bool>(var.value) == false)
	{
		tree->next(); // {

		while (true)
		{
			auto& _token = tree->get();
			if (auto value = _token.getValue()) // }
				break;
							
			tree->next();
		}

		if (!tree->next()) // no else
			tree->up();
	}
}

Shell::Block& Shell::openBlock()
{
	return m_blocks.emplace_back();
}

void Shell::closeBlock()
{
	m_blocks.pop_back();
}
	
Shell::Variable& Shell::declare(string const& id, VType type)
{
	auto& vars = m_blocks.back();

	try { search(id); }
	catch (runtime_error&)
	{
		auto& var = vars.emplace_back();
		var.id    = id;
		var.type  = type;
		initialize(var);
		
		return var;
	}

	throw runtime_error(""); // repeated declaration of the variable
}

// Temporary variable
Shell::Variable& Shell::declare()
{
	auto& vars = m_blocks.back();

	auto& var = vars.emplace_back();
	var.type  = VType::Any;

	return var;
}

// Temporary variable
Shell::Variable& Shell::declare(VType type)
{
	auto& vars = m_blocks.back();

	auto& var = vars.emplace_back();
	var.type  = type;
	initialize(var);
		
	return var;
}

// Temporary variable
Shell::Variable& Shell::declare(VType type, string const& value)
{
	auto& vars = m_blocks.back();

	auto& var = vars.emplace_back();
	var.type  = type;
	initialize(var, value);
		
	return var;
}

// Temporary variable
Shell::Variable& Shell::declare(VType type, Value const& value)
{
	auto& vars = m_blocks.back();

	auto& var = vars.emplace_back();
	var.type  = type;
	var.value = value;

	return var;
}
	
bool Shell::stob(string const& value) const
{
	if (value.front() == 't' || value.front() == '1')
		return true;

	return false;
}

Shell::VType Shell::toVIType(TkValueType type) const
{
	switch (type)
	{
	case TkValueType::BoolLiteral:
		return VType::Bool;

	case TkValueType::FloatLiteral:
		return VType::Float;

	case TkValueType::IntLiteral:
		return VType::Int;

	case TkValueType::StringLiteral:
		return VType::String;

	default:
		throw runtime_error("");
	}
}

Shell::VType Shell::toVIType(char type) const
{
	switch (type)
	{
	case 'b':
		return VType::Bool;

	case 'f':
		return VType::Float;

	case 'i':
		return VType::Int;

	case 's':
		return VType::String;

	default:
		throw runtime_error("");
	}
}

void Shell::initialize(Variable& var)
{
	switch (var.type)
	{
	case VType::Bool:
		var.value.emplace<bool>(false);
	break;

	case VType::Float:
		var.value.emplace<float>(0.0f);
	break;

	case VType::Int:
		var.value.emplace<int>(0);
	break;

	case VType::String:
		var.value.emplace<string>();
	break;

	default:
		break;
	}
}

void Shell::initialize(Variable& var, string const& value)
{
	switch (var.type)
	{
	case VType::Bool:
		var.value.emplace<bool>(stob(value));
	break;

	case VType::Float:
		var.value.emplace<float>(stof(value));
	break;

	case VType::Int:
		var.value.emplace<int>(stoi(value));
	break;

	case VType::String:
		var.value = value;
	break;

	default:
		break;
	}
}

void Shell::unaryOp(char op, Variable& target)
{
	auto& tvalue = target.value;

	switch (op)
	{
	case '~':
	{
		switch (target.type)
		{
		case VType::Float:
		{
			auto& value = get<float>(tvalue);
			value = -value;
		}
		break;

		case VType::Int:
		{
			auto& value = get<int>(tvalue);
			value = -value;
		}
		break;

		default:
			break;
		}
	}
	break;

	case '!':
	{
		if (target.type == VType::Bool)
		{
			auto& value = get<bool>(tvalue);
			value = !value;
		}
	}
	break;
	}
}

void Shell::arithmOp(char op, Variable& target, Variable& lvar, Value const& rvalue)
{
	if (target.type == VType::Any)
	{
		target.type = lvar.type;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;

	switch (op)
	{
	case '^':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += pow(get<float>(lvalue), get<float>(rvalue));
		break;

		case VType::Int:
			get<int>(tvalue) += (int)pow(get<int>(lvalue), get<int>(rvalue));
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '*':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) * get<float>(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) * get<int>(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '/':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) / get<float>(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) / get<int>(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '%':
	{
		switch (lvar.type)
		{
		case VType::Float:
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) % get<int>(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '+':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) + get<float>(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) + get<int>(rvalue);
		break;

		case VType::String:
			get<string>(tvalue) += get<string>(lvalue) + get<string>(rvalue);
		break;

		default:
			break;
		}
	}
	break;

	case '-':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) - get<float>(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) - get<int>(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;
	}
}

void Shell::arithmOp(char op, Variable& target, Variable& lvar, string const& rvalue)
{
	if (target.type == VType::Any)
	{
		target.type = lvar.type;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;

	switch (op)
	{
	case '^':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += pow(get<float>(lvalue), stof(rvalue));
		break;

		case VType::Int:
			get<int>(tvalue) += (int)pow(get<int>(lvalue), stoi(rvalue));
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '*':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) * stof(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) * stoi(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '/':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) / stof(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) / stoi(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '%':
	{
		switch (lvar.type)
		{
		case VType::Float:
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) % stoi(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;

	case '+':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) + stof(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) + stoi(rvalue);
		break;

		case VType::String:
			get<string>(tvalue) += get<string>(lvalue) + rvalue;
		break;

		default:
			break;
		}
	}
	break;

	case '-':
	{
		switch (lvar.type)
		{
		case VType::Float:
			get<float>(tvalue) += get<float>(lvalue) - stof(rvalue);
		break;

		case VType::Int:
			get<int>(tvalue) += get<int>(lvalue) - stoi(rvalue);
		break;

		case VType::String:
		break;

		default:
			break;
		}
	}
	break;
	}
}

void Shell::logicOp(string const& op, Variable& target, Variable& lvar, Value const& rvalue)
{
	if (target.type != VType::Bool)
	{
		target.type = VType::Bool;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;

	auto len = op.size();

	switch (op[0])
	{
	case '<':
	{
		if (len > 1ull)
			tvalue = lvalue <= rvalue;

		else
			tvalue = lvalue < rvalue;
	}
	break;

	case '>':
	{
		if (len > 1ull)
			tvalue = lvalue >= rvalue;

		else
			tvalue = lvalue > rvalue;
	}
	break;

	case '=':
	{
		tvalue = lvalue == rvalue;
	}
	break;

	case '!':
	{
		tvalue = lvalue != rvalue;
	}
	break;

	case '&':
	{
		if (lvar.type == VType::Bool)
			tvalue = get<bool>(lvalue) && get<bool>(rvalue);
	}
	break;

	case '|':
	{
		if (lvar.type == VType::Bool)
			tvalue = get<bool>(lvalue) || get<bool>(rvalue);
	}
	break;
	}
}

void Shell::logicOp(string const& op, Variable& target, Variable& lvar, string const& rvalue)
{
	if (target.type == VType::Any)
	{
		target.type = VType::Bool;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;

	auto len = op.size();

	switch (op[0])
	{
	case '<':
	{
		if (len > 1ull)
			switch (lvar.type)
			{
			case VType::Float:
				tvalue = get<float>(lvalue) <= stof(rvalue);
			break;

			case VType::Int:
				tvalue = get<int>(lvalue) <= stoi(rvalue);
			break;

			case VType::String:
			break;
					
			default:
				break;
			}

		else
			switch (lvar.type)
			{
			case VType::Float:
				tvalue = get<float>(lvalue) < stof(rvalue);
			break;

			case VType::Int:
				tvalue = get<int>(lvalue) < stoi(rvalue);
			break;

			case VType::String:
			break;
					
			default:
				break;
			}
	}
	break;

	case '>':
	{
		if (len > 1ull)
			switch (lvar.type)
			{
			case VType::Float:
				tvalue = get<float>(lvalue) >= stof(rvalue);
			break;

			case VType::Int:
				tvalue = get<int>(lvalue) >= stoi(rvalue);
			break;

			case VType::String:
			break;
					
			default:
				break;
			}

		else
			switch (lvar.type)
			{
			case VType::Float:
				tvalue = get<float>(lvalue) > stof(rvalue);
			break;

			case VType::Int:
				tvalue = get<int>(lvalue) > stoi(rvalue);
			break;

			case VType::String:
			break;
					
			default:
				break;
			}
	}
	break;

	case '=':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			tvalue = get<bool>(lvalue) == stob(rvalue);
		break;

		case VType::Float:
			tvalue = get<float>(lvalue) == stof(rvalue);
		break;

		case VType::Int:
			tvalue = get<int>(lvalue) == stoi(rvalue);
		break;

		case VType::String:
			tvalue = get<string>(lvalue) == rvalue;
		break;
					
		default:
			break;
		}
	}
	break;

	case '!':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			tvalue = get<bool>(lvalue) != stob(rvalue);
		break;

		case VType::Float:
			tvalue = get<float>(lvalue) != stof(rvalue);
		break;

		case VType::Int:
			tvalue = get<int>(lvalue) != stoi(rvalue);
		break;

		case VType::String:
			tvalue = get<string>(lvalue) != rvalue;
		break;
					
		default:
			break;
		}
	}
	break;

	case '&':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			tvalue = get<bool>(lvalue) && stob(rvalue);
		break;
					
		default:
			break;
		}
	}
	break;

	case '|':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			tvalue = get<bool>(lvalue) || stob(rvalue);
		break;
					
		default:
			break;
		}
	}
	break;
	}
}

Shell::ValuePtr Shell::toValue(Token::Value const& tokenValue) const
{
	auto  type  = toVIType(tokenValue.type);
	auto& text  = tokenValue.text;
	auto  value = make_unique<Value>();

	switch (type)
	{
	case VType::Bool:
		value->emplace<bool>(stob(text));
	break;

	case VType::Float:
		value->emplace<float>(stof(text));
	break;

	case VType::Int:
		value->emplace<int>(stoi(text));
	break;

	case VType::String:
		value->emplace<string>(text);
	break;

	default:
		break;
	}

	return value;
}

Shell::Expression Shell::toPostfix(Expression const& infix) const
{
	// Функция определения приоритета операторов
	auto precedence = [](char op)
	{
		if (op == '|')
			return 1;

		if (op == '&')
			return 2;

		if (op == '=' || op == '!')
			return 3;

		if (op == '<' || op == '>')
			return 4;

		if (op == '+' || op == '-')
			return 5;

		if (op == '*' || op == '/' || op == '%')
			return 6;

		if (op == '~' || op == '!')
			return 7;

		if (op == '^')
			return 8;

		return 0;
	};

	stack<Token::Value const*>  operators;
	list<Token::Value const*> output;

	for (auto token : infix)
	{
		auto& text = token->text;
		auto  pres = precedence(text[0]);

		if (token->type != TkValueType::Operator)
			output.push_back(token);

		else if (text[0] == '(')
			operators.push(token);

		else if (text[0] == ')')
		{
			while (!operators.empty() && operators.top()->text[0] != '(')
			{
				output.push_back(operators.top());
				operators.pop();
			}

			if (!operators.empty())
				operators.pop(); // Убрать '(' из стека
		} 
		// Если токен - оператор
		else if (pres > 0)
		{
			while (!operators.empty() && precedence(operators.top()->text[0]) >= pres)
			{
				output.push_back(operators.top());

				operators.pop();
			}

			operators.push(token);
		}
	}

    // Удаление оставшихся операторов из стека
	while (!operators.empty())
	{
		output.push_back(operators.top());

        operators.pop();
	}

	return output;
}