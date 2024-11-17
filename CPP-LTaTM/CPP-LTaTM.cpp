#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace std;

const unordered_set<string> TYPES
{
    "int", "float", "string", "bool"
};

const unordered_set<string> OPERATORS
{
    "(", ")", "^", "!", "-", "+", "*", "/", "%", "<", ">", "<=", ">=",
    "==", "!=", "&&", "||", "print", "="
};

enum class WordType
{
    Id = 0,
    Type,
    Operator,
    Literal,
    Separator
};

bool getChar(stringstream& input, char& buf)
{
    if (input.eof())
        return false;

    input.get(buf);

    return true;
}

WordType detectType(string const& str)
{
    if (TYPES.contains(str))
        return WordType::Type;

    else if (OPERATORS.contains(str))
        return WordType::Operator;
}

void printAndClear(WordType type, ostringstream& out)
{
    switch (type)
    {
    case WordType::Id: cout << "Идентификатор: ";
        break;
    case WordType::Type: cout << "Тип: ";
        break;
    case WordType::Operator: cout << "Оператор: ";
        break;
    case WordType::Literal: cout << "Литерал: ";
        break;
    case WordType::Separator: cout << "Разделитель: ";
        break;
    default:
        break;
    }

    cout << out.str() << '\n';
    out.clear();
}

int main()
{
    ifstream     file("input.txt", ios::in);
    stringstream input;

    input << file.rdbuf();
    file.close();

    char buf;

    ostringstream out;

    while (getChar(input, buf))
    {
        out << buf;

        if (buf == '{')
            printAndClear(WordType::Separator, out);

        else if (buf == '}')
            printAndClear(WordType::Separator, out);

        else if (buf == ';')
            printAndClear(WordType::Separator, out);

        else if (buf == '(')
            printAndClear(WordType::Operator, out);

        else if (buf == ')')
            printAndClear(WordType::Operator, out);

        else if (buf == '+')
            printAndClear(WordType::Operator, out);

        else if (buf == '-')
            printAndClear(WordType::Operator, out);

        else if (buf == '/')
            printAndClear(WordType::Operator, out);

        else if (buf == '%')
            printAndClear(WordType::Operator, out);

        else if (buf == '^')
            printAndClear(WordType::Operator, out);

        else if (buf == '!')
        {
            printAndClear(WordType::Operator, out);
        }

        else if (buf == '!=')
            printAndClear(WordType::Operator, out);

        else if (buf == '==')
            printAndClear(WordType::Operator, out);

        else if (buf == '!')
            printAndClear(WordType::Operator, out);

        else if (isalpha(buf))
        {

        }
    }
}