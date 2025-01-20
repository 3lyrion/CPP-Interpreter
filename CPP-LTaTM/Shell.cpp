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

			//case Token::Type::Expression:
			//	expression();
			//break;

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

Shell::ValueInfo& Shell::search(string const& id)
{
	for (auto& block : m_blocks)
	{
		auto& ids = block.ids;

		auto entry = find(ids.cbegin(), ids.cend(), id);
		if (entry != ids.cend())
	//		return block.infos[distance(ids.cbegin(), entry)];
		{
			auto it = block.infos.begin();
			advance(it, distance(ids.cbegin(), entry));

			return *it;
		}
	}

	throw runtime_error("");
}

void Shell::declaration()
{
	tree->down();

	auto type = toVIType(tree->get().getValue()->text[0]);

	tree->next(); // type

	auto& id = tree->get().getValue()->text;

	auto& info_id = declare(id, type);

	if (tree->next()) // id
	{
		tree->next(); // =

		expression(info_id);
	}

	tree->up();
}

void Shell::expression(ValueInfo& target)
{
	tree->down();

	auto& back  = openBlock();
	auto& infos = back.infos;

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
				auto rhs = move(infos.back()); infos.pop_back();
				auto lhs = move(infos.back()); infos.pop_back();

				auto& var = declare();

				auto& _text = value->text;

				if (_text[0] == '<' || _text[0] == '>' || _text.size() == 2ull)
					logicOp(_text, var, lhs, rhs.value);

				else
					arithmOp(_text[0], var, lhs, rhs.value);
			}

		}

		target = infos.back();
	}

	tree->up();
	closeBlock();

//	auto result = closeBlock();

	//auto target.value = m_blocks.back().target;
	//if (target)
	//{
	//	if (target->type == result.type)
	//		target->value = result.value;

	//	else if (target->type == VIType::Any)
	//	{
	//		target->type  = result.type;
	//		target->value = result.value;
	//	}
	//		
	//	else
	//		throw runtime_error("");

	//	target.value = nullptr;
	//}
}

void Shell::expressionStatement()
{
	tree->down();

	auto& id = tree->get().getValue()->text;

	auto& info_id = search(id);

	tree->next(); // id

	tree->next(); // =

	expression(info_id);

	tree->up();
}

void Shell::iterationStatement()
{
	tree->down();

	tree->next(); // while
	tree->next(); // (

	auto& var = declare(VIType::Bool);
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
	case VIType::Bool:
		cout << get<bool>(var.value);
	break;

	case VIType::Float:
		cout << get<float>(var.value);
	break;

	case VIType::Int:
		cout << get<int>(var.value);
	break;

	case VIType::String:
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

	auto& var = declare(VIType::Bool);
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
	
Shell::ValueInfo& Shell::declare(string const& id, VIType type)
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
	initialize(info);
		
	return info;
}

// Temporary variable
Shell::ValueInfo& Shell::declare()
{
	auto& back   = m_blocks.back();
	auto& ids   = back.ids;
	auto& infos = back.infos;

	ids.emplace_back();
	auto& info = infos.emplace_back();
	info.type  = VIType::Any;

	return info;
}

// Temporary variable
Shell::ValueInfo& Shell::declare(VIType type)
{
	auto& back   = m_blocks.back();
	auto& ids   = back.ids;
	auto& infos = back.infos;

	ids.emplace_back();
	auto& info = infos.emplace_back();
	info.type  = type;

	initialize(info);
		
	return info;
}

// Temporary variable
Shell::ValueInfo& Shell::declare(VIType type, string const& value)
{
	auto& back   = m_blocks.back();
	auto& ids   = back.ids;
	auto& infos = back.infos;

	ids.emplace_back();
	auto& info = infos.emplace_back();
	info.type  = type;

	initialize(info, value);
		
	return info;
}

// Temporary variable
Shell::ValueInfo& Shell::declare(VIType type, Value const& value)
{
	auto& back   = m_blocks.back();
	auto& ids   = back.ids;
	auto& infos = back.infos;

	ids.emplace_back();
	auto& info = infos.emplace_back();
	info.type  = type;
	info.value = value;

	return info;
}
	
bool Shell::stob(string const& value) const
{
	if (value.front() == 't' || value.front() == '1')
		return true;

	return false;
}

Shell::VIType Shell::toVIType(TkValueType type) const
{
	switch (type)
	{
	case TkValueType::BoolLiteral:
		return VIType::Bool;

	case TkValueType::FloatLiteral:
		return VIType::Float;

	case TkValueType::IntLiteral:
		return VIType::Int;

	case TkValueType::StringLiteral:
		return VIType::String;

	default:
		throw runtime_error("");
	}
}

Shell::VIType Shell::toVIType(char type) const
{
	switch (type)
	{
	case 'b':
		return VIType::Bool;

	case 'f':
		return VIType::Float;

	case 'i':
		return VIType::Int;

	case 's':
		return VIType::String;

	default:
		throw runtime_error("");
	}
}

void Shell::initialize(ValueInfo& info)
{
	switch (info.type)
	{
	case VIType::Bool:
		info.value.emplace<bool>(false);
	break;

	case VIType::Float:
		info.value.emplace<float>(0.0f);
	break;

	case VIType::Int:
		info.value.emplace<int>(0);
	break;

	case VIType::String:
		info.value.emplace<string>();
	break;

	default:
		break;
	}
}

void Shell::initialize(ValueInfo& info, string const& value)
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
		info.value = value;
	break;

	default:
		break;
	}
}

void Shell::arithmOp(char op, ValueInfo& target, ValueInfo& linfo, Value const& rvalue)
{
	if (target.type == VIType::Any)
	{
		target.type = linfo.type;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = linfo.value;

	switch (op)
	{
	case '^':
	{
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += pow(get<float>(lvalue), get<float>(rvalue));
		break;

		case VIType::Int:
			get<int>(tvalue) += (int)pow(get<int>(lvalue), get<int>(rvalue));
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) * get<float>(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) * get<int>(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) / get<float>(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) / get<int>(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) % get<int>(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) + get<float>(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) + get<int>(rvalue);
		break;

		case VIType::String:
			get<string>(tvalue) += get<string>(lvalue) + get<string>(rvalue);
		break;

		default:
			break;
		}
	}
	break;

	case '-':
	{
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) - get<float>(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) - get<int>(rvalue);
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

void Shell::arithmOp(char op, ValueInfo& target, ValueInfo& linfo, string const& rvalue)
{
	if (target.type == VIType::Any)
	{
		target.type = linfo.type;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = linfo.value;

	switch (op)
	{
	case '^':
	{
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += pow(get<float>(lvalue), stof(rvalue));
		break;

		case VIType::Int:
			get<int>(tvalue) += (int)pow(get<int>(lvalue), stoi(rvalue));
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) * stof(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) * stoi(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) / stof(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) / stoi(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) % stoi(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) + stof(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) + stoi(rvalue);
		break;

		case VIType::String:
			get<string>(tvalue) += get<string>(lvalue) + rvalue;
		break;

		default:
			break;
		}
	}
	break;

	case '-':
	{
		switch (linfo.type)
		{
		case VIType::Float:
			get<float>(tvalue) += get<float>(lvalue) - stof(rvalue);
		break;

		case VIType::Int:
			get<int>(tvalue) += get<int>(lvalue) - stoi(rvalue);
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

void Shell::logicOp(string const& op, ValueInfo& target, ValueInfo& linfo, Value const& rvalue)
{
	if (target.type != VIType::Bool)
	{
		target.type = VIType::Bool;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = linfo.value;

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
		if (linfo.type == VIType::Bool)
			tvalue = get<bool>(lvalue) && get<bool>(rvalue);
	}
	break;

	case '|':
	{
		if (linfo.type == VIType::Bool)
			tvalue = get<bool>(lvalue) || get<bool>(rvalue);
	}
	break;
	}
}

void Shell::logicOp(string const& op, ValueInfo& target, ValueInfo& linfo, string const& rvalue)
{
	if (target.type == VIType::Any)
	{
		target.type = VIType::Bool;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = linfo.value;

	auto len = op.size();

	switch (op[0])
	{
	case '<':
	{
		if (len > 1ull)
			switch (linfo.type)
			{
			case VIType::Float:
				tvalue = get<float>(lvalue) <= stof(rvalue);
			break;

			case VIType::Int:
				tvalue = get<int>(lvalue) <= stoi(rvalue);
			break;

			case VIType::String:
			break;
					
			default:
				break;
			}

		else
			switch (linfo.type)
			{
			case VIType::Float:
				tvalue = get<float>(lvalue) < stof(rvalue);
			break;

			case VIType::Int:
				tvalue = get<int>(lvalue) < stoi(rvalue);
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
			switch (linfo.type)
			{
			case VIType::Float:
				tvalue = get<float>(lvalue) >= stof(rvalue);
			break;

			case VIType::Int:
				tvalue = get<int>(lvalue) >= stoi(rvalue);
			break;

			case VIType::String:
			break;
					
			default:
				break;
			}

		else
			switch (linfo.type)
			{
			case VIType::Float:
				tvalue = get<float>(lvalue) > stof(rvalue);
			break;

			case VIType::Int:
				tvalue = get<int>(lvalue) > stoi(rvalue);
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
		switch (linfo.type)
		{
		case VIType::Bool:
			tvalue = get<bool>(lvalue) == stob(rvalue);
		break;

		case VIType::Float:
			tvalue = get<float>(lvalue) == stof(rvalue);
		break;

		case VIType::Int:
			tvalue = get<int>(lvalue) == stoi(rvalue);
		break;

		case VIType::String:
			tvalue = get<string>(lvalue) == rvalue;
		break;
					
		default:
			break;
		}
	}
	break;

	case '!':
	{
		switch (linfo.type)
		{
		case VIType::Bool:
			tvalue = get<bool>(lvalue) != stob(rvalue);
		break;

		case VIType::Float:
			tvalue = get<float>(lvalue) != stof(rvalue);
		break;

		case VIType::Int:
			tvalue = get<int>(lvalue) != stoi(rvalue);
		break;

		case VIType::String:
			tvalue = get<string>(lvalue) != rvalue;
		break;
					
		default:
			break;
		}
	}
	break;

	case '&':
	{
		switch (linfo.type)
		{
		case VIType::Bool:
			tvalue = get<bool>(lvalue) && stob(rvalue);
		break;
					
		default:
			break;
		}
	}
	break;

	case '|':
	{
		switch (linfo.type)
		{
		case VIType::Bool:
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
	case VIType::Bool:
		value->emplace<bool>(stob(text));
	break;

	case VIType::Float:
		value->emplace<float>(stof(text));
	break;

	case VIType::Int:
		value->emplace<int>(stoi(text));
	break;

	case VIType::String:
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

		if (op == '^')
			return 7;

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