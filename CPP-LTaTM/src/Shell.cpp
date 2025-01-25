#include <Shell.h>

#define VALIDATE_COMPATIBILITY(OP) if (rvar.type != lvar.type) throwIncompatibilityError(lvar, rvar, OP);

#define VALIDATE_CONVERSION(EXPRESSION, OP) try { EXPRESSION; } catch (exception& e) { throwConversionError(lvar, rvalue, OP, e); }

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

	throw logic_error("");
}

void Shell::declaration()
{
	tree->down();

	bool constant = false;

	auto& first = *tree->get().getValue();
	if (first.text == "const")
	{
		constant = true;
		tree->next();
	}

	auto type = toVType(tree->get().getValue()->text[0]);

	tree->next(); // type

	auto& id = tree->get().getValue()->text;

	auto& var_id = declare(id, type, constant);

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
		// Creating a copy of the variable
		if (value.type == TkValueType::Id)
		{
			auto& v		= search(value.text);
			auto& res	= declare(v.type, v.value);
			res.id		= v.id; // more info for the exceptions

			return res;
		}

		// Creating a variable from the literal
		else
			return declare(toVType(value.type), value.text);
	};

	if (rpn.size() == 1ull)
	{
		auto& value = *rpn.front();

		if (value.type == TkValueType::Id)
			target = search(value.text);

		else
		{
			auto vtype = toVType(value.type);

			if (target.type == VType::Any || target.type == vtype)
			{
				target.type = vtype;
				initialize(target, value.text);
			}

			else
				throwIncompatibilityError(target, value.text, "=");
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
						logicOp(text, var, lhs, rhs);

					else
						arithmOp(text[0], var, lhs, rhs);
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

	auto& var = search(id);
	if (var.constant)
		throwIncompatibilityError(var, "=");

	tree->next(); // id

	tree->next(); // =

	expression(var);

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
	
Shell::Variable& Shell::declare(string const& id, VType type, bool constant)
{
	auto& vars = m_blocks.back();

	try { search(id); }
	catch (logic_error&)
	{
		auto& var		= vars.emplace_back();
		var.id			= id;
		var.type		= type;
		var.constant	= constant;
		initialize(var);
		
		return var;
	}

	throwError("Repeated declaration of the variable: '" + id + '\'');
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

Shell::VType Shell::toVType(TkValueType type) const
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
		throw exception("");
	}
}

Shell::VType Shell::toVType(char type) const
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
		throw exception("");
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
	try
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
	
	catch (exception& e)
	{
		throwConversionError(var, value, "=", e);
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
			throwIncompatibilityError(target, "-");
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

		else
			throwIncompatibilityError(target, "!");
	}
	break;
	}
}

void Shell::arithmOp(char op, Variable& target, Variable& lvar, Variable const& rvar)
{
	if (target.type == VType::Any)
	{
		target.type = lvar.type;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;
	auto& rvalue = rvar.value;

	switch (op)
	{
	case '^':
	{
		switch (lvar.type)
		{
		case VType::Float:
		{
			VALIDATE_COMPATIBILITY("^")
			get<float>(tvalue) += powf(get<float>(lvalue), get<float>(rvalue));
		}
		break;

		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("^")
			get<int>(tvalue) += (int)pow(get<int>(lvalue), get<int>(rvalue));
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "^");
		break;
		}
	}
	break;

	case '*':
	{
		switch (lvar.type)
		{
		case VType::Float:
		{
			VALIDATE_COMPATIBILITY("*")
			get<float>(tvalue) += get<float>(lvalue) * get<float>(rvalue);
		}
		break;

		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("*")
			get<int>(tvalue) += get<int>(lvalue) * get<int>(rvalue);
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "*");
		break;
		}
	}
	break;

	case '/':
	{
		switch (lvar.type)
		{
		case VType::Float:
		{
			VALIDATE_COMPATIBILITY("/")
			get<float>(tvalue) += get<float>(lvalue) / get<float>(rvalue);
		}
		break;

		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("/")
			get<int>(tvalue) += get<int>(lvalue) / get<int>(rvalue);
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "/");
		break;
		}
	}
	break;

	case '%':
	{
		switch (lvar.type)
		{
		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("%")
			get<int>(tvalue) += get<int>(lvalue) % get<int>(rvalue);
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "%");
		break;
		}
	}
	break;

	case '+':
	{
		switch (lvar.type)
		{
		case VType::Float:
		{
			VALIDATE_COMPATIBILITY("+")
			get<float>(tvalue) += get<float>(lvalue) + get<float>(rvalue);
		}
		break;

		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("+")
			get<int>(tvalue) += get<int>(lvalue) + get<int>(rvalue);
		}
		break;

		case VType::String:
		{
			VALIDATE_COMPATIBILITY("+")
			get<string>(tvalue) += get<string>(lvalue) + get<string>(rvalue);
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "+");
		break;
		}
	}
	break;

	case '-':
	{
		switch (lvar.type)
		{
		case VType::Float:
		{
			VALIDATE_COMPATIBILITY("-")
			get<float>(tvalue) += get<float>(lvalue) - get<float>(rvalue);
		}
		break;

		case VType::Int:
		{
			VALIDATE_COMPATIBILITY("-")
			get<int>(tvalue) += get<int>(lvalue) - get<int>(rvalue);
		}
		break;

		default:
			throwIncompatibilityError(lvar, rvar, "-");
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
			VALIDATE_CONVERSION(get<float>(tvalue) += pow(get<float>(lvalue), stof(rvalue)),
				"^")
		break;

		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += (int)pow(get<int>(lvalue), stoi(rvalue)), 
				"^")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "^");
		break;
		}
	}
	break;

	case '*':
	{
		switch (lvar.type)
		{
		case VType::Float:
			VALIDATE_CONVERSION(get<float>(tvalue) += get<float>(lvalue) * stof(rvalue),
				"*")
		break;

		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += get<int>(lvalue) * stoi(rvalue),
				"*")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "*");
		break;
		}
	}
	break;

	case '/':
	{
		switch (lvar.type)
		{
		case VType::Float:
			VALIDATE_CONVERSION(get<float>(tvalue) += get<float>(lvalue) / stof(rvalue),
				"/")
		break;

		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += get<int>(lvalue) / stoi(rvalue),
				"/")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "/");
		break;
		}
	}
	break;

	case '%':
	{
		switch (lvar.type)
		{
		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += get<int>(lvalue) % stoi(rvalue),
				"%")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "%");
		break;
		}
	}
	break;

	case '+':
	{
		switch (lvar.type)
		{
		case VType::Float:
			VALIDATE_CONVERSION(get<float>(tvalue) += get<float>(lvalue) + stof(rvalue),
				"+")
		break;

		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += get<int>(lvalue) + stoi(rvalue),
				"+")
		break;

		case VType::String:
			VALIDATE_CONVERSION(get<string>(tvalue) += get<string>(lvalue) + rvalue,
				"+")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "+");
		break;
		}
	}
	break;

	case '-':
	{
		switch (lvar.type)
		{
		case VType::Float:
			VALIDATE_CONVERSION(get<float>(tvalue) += get<float>(lvalue) - stof(rvalue),
				"-")
		break;

		case VType::Int:
			VALIDATE_CONVERSION(get<int>(tvalue) += get<int>(lvalue) - stoi(rvalue),
				"-")
		break;

		default:
			throwIncompatibilityError(lvar, rvalue, "-");
		break;
		}
	}
	break;
	}
}

void Shell::logicOp(string const& op, Variable& target, Variable& lvar, Variable const& rvar)
{
	if (target.type != VType::Bool)
	{
		target.type = VType::Bool;
		initialize(target);
	}

	auto& tvalue = target.value;
	auto& lvalue = lvar.value;
	auto& rvalue = rvar.value;

	auto len = op.size();

	switch (op[0])
	{
	case '<':
	{
		VALIDATE_COMPATIBILITY(op)

		if (len > 1ull)
			tvalue = lvalue <= rvalue;

		else
			tvalue = lvalue < rvalue;
	}
	break;

	case '>':
	{
		VALIDATE_COMPATIBILITY(op)

		if (len > 1ull)
			tvalue = lvalue >= rvalue;

		else
			tvalue = lvalue > rvalue;
	}
	break;

	case '=':
	{
		VALIDATE_COMPATIBILITY(op)
		tvalue = lvalue == rvalue;
	}
	break;

	case '!':
	{
		VALIDATE_COMPATIBILITY(op)
		tvalue = lvalue != rvalue;
	}
	break;

	case '&':
	{
		VALIDATE_COMPATIBILITY(op)
		tvalue = get<bool>(lvalue) && get<bool>(rvalue);
	}
	break;

	case '|':
	{
		VALIDATE_COMPATIBILITY(op)
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
				VALIDATE_CONVERSION(tvalue = get<float>(lvalue) <= stof(rvalue),
					op)
			break;

			case VType::Int:
				VALIDATE_CONVERSION(tvalue = get<int>(lvalue) <= stoi(rvalue),
					op)
			break;
					
			default:
				throwIncompatibilityError(lvar, rvalue, "<");
				break;
			}

		else
			switch (lvar.type)
			{
			case VType::Float:
				VALIDATE_CONVERSION(tvalue = get<float>(lvalue) < stof(rvalue);,
					op)
			break;

			case VType::Int:
				VALIDATE_CONVERSION(tvalue = get<int>(lvalue) < stoi(rvalue),
					op)
			break;
					
			default:
				throwIncompatibilityError(lvar, rvalue, "<=");
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
				VALIDATE_CONVERSION(tvalue = get<float>(lvalue) >= stof(rvalue),
					op)
			break;

			case VType::Int:
				VALIDATE_CONVERSION(tvalue = get<int>(lvalue) >= stoi(rvalue),
					op)
			break;
					
			default:
				throwIncompatibilityError(lvar, rvalue, ">=");
				break;
			}

		else
			switch (lvar.type)
			{
			case VType::Float:
				VALIDATE_CONVERSION(tvalue = get<float>(lvalue) > stof(rvalue),
					op)
			break;

			case VType::Int:
				VALIDATE_CONVERSION(tvalue = get<int>(lvalue) > stoi(rvalue),
					op)
			break;
					
			default:
				throwIncompatibilityError(lvar, rvalue, ">");
				break;
			}
	}
	break;

	case '=':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			VALIDATE_CONVERSION(tvalue = get<bool>(lvalue) == stob(rvalue),
				op)
		break;

		case VType::Float:
			VALIDATE_CONVERSION(tvalue = get<float>(lvalue) == stof(rvalue),
				op)
		break;

		case VType::Int:
			VALIDATE_CONVERSION(tvalue = get<int>(lvalue) == stoi(rvalue),
				op)
		break;

		case VType::String:
			VALIDATE_CONVERSION(tvalue = get<string>(lvalue) == rvalue,
				op)
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
			VALIDATE_CONVERSION(tvalue = get<bool>(lvalue) != stob(rvalue),
				op)
		break;

		case VType::Float:
			VALIDATE_CONVERSION(tvalue = get<float>(lvalue) != stof(rvalue),
				op)
		break;

		case VType::Int:
			VALIDATE_CONVERSION(tvalue = get<int>(lvalue) != stoi(rvalue),
				op)
		break;

		case VType::String:
			VALIDATE_CONVERSION(tvalue = get<string>(lvalue) != rvalue,
				op)
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
			VALIDATE_CONVERSION(get<bool>(lvalue) && stob(rvalue),
				op)
		break;
					
		default:
			throwIncompatibilityError(lvar, rvalue, "&");
			break;
		}
	}
	break;

	case '|':
	{
		switch (lvar.type)
		{
		case VType::Bool:
			VALIDATE_CONVERSION(get<bool>(lvalue) || stob(rvalue),
				op)
		break;
					
		default:
			throwIncompatibilityError(lvar, rvalue, "|");
			break;
		}
	}
	break;
	}
}

Shell::ValuePtr Shell::toValue(Token::Value const& tokenValue) const
{
	auto  type  = toVType(tokenValue.type);
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
				operators.pop(); // '('
		} 
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

	while (!operators.empty())
	{
		output.push_back(operators.top());

        operators.pop();
	}

	return output;
}

string Shell::getType(Variable const& var) const
{
	string out = "bool";

	switch (var.type)
	{
	case VType::Float:
		out = "float";
	break;

	case VType::Int:
		out = "int";
	break;

	case VType::String:
		out = "string";
	break;
		
	default:
		break;
	}

	if (var.constant)
		out.insert(0, "const ");

	return out;
}

string Shell::getValue(Variable const& var) const
{
	switch (var.type)
	{
	case VType::Float:	return to_string(get<float>(var.value));
	case VType::Int:	return to_string(get<int>(var.value));
	case VType::String:	return get<string>(var.value);
	default:			return to_string(get<bool>(var.value));
	}
}

void Shell::throwError(string const& msg) const
{
	printf("\n%s\n", msg.c_str());

	system("pause");
	exit(EXIT_FAILURE);
}

void Shell::throwConversionError(Variable const& lvar, string const& rvalue, string const& op, exception const& e) const
{
	auto print_stack_trace = [&]
	{
		if (lvar.id)
			printf(">>> Stack trace: %s %s %s\n", lvar.id->c_str(), op.c_str(), rvalue.c_str());

		else
			cout << ">>> Stack trace: "
				    << getValue(lvar)	<< ' '
				    << op				<< ' '
				    << rvalue			<< '\n';
	};

	try
	{
		dynamic_cast<invalid_argument const&>(e);

		printf("\nThe argument is invalid: %s\n", rvalue.c_str());
		print_stack_trace();
						
	}
	catch (bad_cast&)
	{
		printf("\nThe argument is out of range: %s\n", rvalue.c_str());
		print_stack_trace();
	}

	system("pause");
	exit(EXIT_FAILURE);
}

void Shell::throwIncompatibilityError(Variable const& var, string const& op) const
{
	auto type = getType(var);

	if (var.id)
		printf("\n'%s' has an incompatible type: %s\n", var.id->c_str(), type.c_str());

	else
		printf("\nThe argument has an incompatible type: %s\n", type.c_str());

	cout << ">>> Stack trace: ";
	if (op[0] != '=')	cout << op				<< ' ';
	if (var.id)			cout << *var.id			<< ' ';
	else				cout << getValue(var)	<< ' ';
	if (op[0] == '=')	cout << op				<< ' ';
	cout << '\n';

	system("pause");
	exit(EXIT_FAILURE);
}

void Shell::throwIncompatibilityError(Variable const& lvar, Variable const& rvar, string const& op) const
{
	auto ltype = getType(lvar);

	if (lvar.id)
		printf("\n'%s' has an incompatible type: %s\n", lvar.id->c_str(), ltype.c_str());

	else
		printf("\nThe argument has an incompatible type: %s\n", ltype.c_str());

	cout << ">>> Stack trace: ";
	if (lvar.id) cout << *lvar.id		<< ' ';
	else         cout << getValue(lvar)	<< ' ';
	cout << op << ' ';
	if (rvar.id) cout << *rvar.id		<< ' ';
	else         cout << getValue(rvar)	<< '\n';

	system("pause");
	exit(EXIT_FAILURE);
}

void Shell::throwIncompatibilityError(Variable const& lvar, string const& rvalue, string const& op) const
{
	auto ltype = getType(lvar);

	if (lvar.id)
		printf("\n'%s' has an incompatible type: %s\n", lvar.id->c_str(), ltype.c_str());

	else
		printf("\nThe argument has an incompatible type: %s\n", ltype.c_str());

	cout << ">>> Stack trace: ";
	if (lvar.id) cout << *lvar.id		<< ' ';
	else         cout << getValue(lvar)	<< ' ';
	cout << op << ' ' << rvalue << '\n';

	system("pause");
	exit(EXIT_FAILURE);
}