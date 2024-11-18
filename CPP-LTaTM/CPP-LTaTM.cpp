#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace std;

const unordered_set<char> SEPARATORS
{
    '}', ')', '^', '-', '+', '*', '/', '%', '<', '>', '<', '>',
    '=', '&', '|', ';', ' ', '\n', '\t', '\r'
};

const unordered_set<char> TEXT_SEPARATORS
{
    ' ', '\n', '\t', '\r'
};

enum class WordType
{
    Id = 0,
    Type,
    Operator,
    Literal,
    Separator,
    Keyword
};

enum class ScanMode
{
    Read = 0,
    Skip,
    Shutdown
};

bool getChar(stringstream& input, char& buf)
{
    if (input.eof())
        return false;

    input.get(buf);

    return true;
}

void flush(WordType type, ostringstream& out)
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
    case WordType::Keyword: cout << "Ключевое слово: ";
        break;
    default:
        break;
    }

    cout << out.str() << '\n';
    out.str("");
}

int main()
{
    setlocale(LC_CTYPE, "Russian");

//    ifstream     file("input.txt", ios::in);

    stringstream input(R"(

        // Начало
        int x = 0;
        float f = 0.0;
        const float F_F3 = 5.25;

        if (x == 0)
        {
            f = 3./2;
        }

        else
        {
            f = .25*3^6;
        }

        /*
        Цикл
        */
        while (true)
        {
            ;
        }

        print "str";            

    )");

 //   input << file.rdbuf();
 //   file.close();

    auto mode = ScanMode::Read;

    char buf;

    ostringstream out;

    while (mode != ScanMode::Shutdown)
    {
        if (mode == ScanMode::Skip)
            mode = ScanMode::Read;

        else if (mode == ScanMode::Read)
        {
            if (!getChar(input, buf)) break;

            if (TEXT_SEPARATORS.contains(buf)) continue;

            out << buf;
        }

        if (buf == '{')
            flush(WordType::Separator, out);

        else if (buf == '}')
            flush(WordType::Separator, out);

        else if (buf == ';')
            flush(WordType::Separator, out);

        else if (buf == '(')
            flush(WordType::Operator, out);

        else if (buf == ')')
            flush(WordType::Operator, out);

        else if (buf == '+')
            flush(WordType::Operator, out);

        else if (buf == '-')
        {
            if (getChar(input, buf))
            {
                if (isdigit(buf))
                {
                    out << buf;

                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (getChar(input, buf) && buf == '.')
                        out << buf;

                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Literal, out);

                        out << buf;
                    }
                }

                else if (buf == '.')
                {
                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Literal, out);

                        out << buf;
                    }
                }

                else flush(WordType::Operator, out);
            }
        }

        else if (buf == '/')
        {
            if (getChar(input, buf))
            {
                if (buf == '/')
                {
                    /*
                    out << buf;

                    flush(WordType::Separator, out);
                    */

                    out.str("");

                    while (getChar(input, buf) && buf != '\n');
                }

                else if (buf == '*')
                {
                    /*
                    out << buf;

                    flush(WordType::Separator, out);
                    */

                    out.str("");

                    comment:

                    while (getChar(input, buf) && buf != '*');

                    if (getChar(input, buf) && buf == '/')
                    {
                        /*
                        out << '*' << buf;

                        flush(WordType::Separator, out);
                        */
                    }

                    else goto comment;
                }

                else
                {
                    flush(WordType::Operator, out);

                    out << buf;

                    mode = ScanMode::Skip;
                }
            }
        }

        else if (buf == '%')
            flush(WordType::Operator, out);

        else if (buf == '^')
            flush(WordType::Operator, out);

        else if (buf == '!')
        {
            if (getChar(input, buf))
            {
                if (buf == '=')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }

                else
                {
                    flush(WordType::Operator, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }
            }
        }

        else if (buf == '=')
        {
            if (getChar(input, buf))
            {
                if (buf == '=')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }

                else
                {
                    flush(WordType::Operator, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }
            }
        }

        else if (buf == '>')
        {
            if (getChar(input, buf))
            {
                if (buf == '=')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }

                else
                {
                    flush(WordType::Operator, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }
            }
        }

        else if (buf == '<')
        {
            if (getChar(input, buf))
            {
                if (buf == '=')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }

                else
                {
                    flush(WordType::Operator, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }
            }
        }

        else if (buf == '&')
        {
            if (getChar(input, buf))
            {
                if (buf == '&')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }
            }
        }

        else if (buf == '|')
        {
            if (getChar(input, buf))
            {
                if (buf == '|')
                {
                    out << buf;

                    flush(WordType::Operator, out);
                }
            }
        }

        else if (isalpha(buf))
        {
            if (buf == 'p')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'r')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'i')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'n')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 't')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (TEXT_SEPARATORS.contains(buf))
                                                        flush(WordType::Operator, out);

                                                    else goto id;
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'c')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'o')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'n')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 's')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 't')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (TEXT_SEPARATORS.contains(buf))
                                                        flush(WordType::Keyword, out);

                                                    else goto id;
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'b')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'o')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'o')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'l')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (TEXT_SEPARATORS.contains(buf))
                                                flush(WordType::Type, out);

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'i')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'n')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 't')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (TEXT_SEPARATORS.contains(buf))
                                        flush(WordType::Type, out);

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else if (buf == 'f')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (TEXT_SEPARATORS.contains(buf))
                                flush(WordType::Operator, out);

                            else if (buf == '(')
                            {
                                flush(WordType::Operator, out);

                                out << buf;

                                mode = ScanMode::Skip;
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'f')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'l')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'o')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'a')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 't')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (TEXT_SEPARATORS.contains(buf))
                                                        flush(WordType::Type, out);

                                                    else goto id;
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else if (buf == 'a')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'l')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 's')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 'e')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (TEXT_SEPARATORS.contains(buf))
                                                        flush(WordType::Literal, out);

                                                    else if (buf == ')' || buf == '}')
                                                    {
                                                        flush(WordType::Literal, out);

                                                        out << buf;

                                                        mode = ScanMode::Skip;
                                                    }

                                                    else goto id;
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 's')
            {
                if (getChar(input, buf))
                {
                    if (buf == 't')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'r')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'i')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 'n')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (buf == 'g')
                                                    {
                                                        out << buf;

                                                        if (getChar(input, buf))
                                                        {
                                                            if (TEXT_SEPARATORS.contains(buf))
                                                                flush(WordType::Type, out);

                                                            else goto id;
                                                        }
                                                    }

                                                    else goto id;
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'e')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'l')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 's')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'e')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (TEXT_SEPARATORS.contains(buf))
                                                flush(WordType::Operator, out);

                                            else if (buf == '(')
                                            {
                                                flush(WordType::Operator, out);

                                                out << buf;

                                                mode = ScanMode::Skip;
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 'w')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'h')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'i')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'l')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (buf == 'e')
                                            {
                                                out << buf;

                                                if (getChar(input, buf))
                                                {
                                                    if (TEXT_SEPARATORS.contains(buf))
                                                        flush(WordType::Literal, out);

                                                    else if (buf == '(')
                                                    {
                                                        flush(WordType::Literal, out);

                                                        out << buf;

                                                        mode = ScanMode::Skip;
                                                    }
                                                }
                                            }

                                            else goto id;
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else if (buf == 't')
            {
                if (getChar(input, buf))
                {
                    if (buf == 'r')
                    {
                        out << buf;

                        if (getChar(input, buf))
                        {
                            if (buf == 'u')
                            {
                                out << buf;

                                if (getChar(input, buf))
                                {
                                    if (buf == 'e')
                                    {
                                        out << buf;

                                        if (getChar(input, buf))
                                        {
                                            if (TEXT_SEPARATORS.contains(buf))
                                                flush(WordType::Literal, out);

                                            else if (buf == ')' || buf == '}')
                                            {
                                                flush(WordType::Literal, out);

                                                out << buf;

                                                mode = ScanMode::Skip;
                                            }
                                        }
                                    }

                                    else goto id;
                                }
                            }

                            else goto id;
                        }
                    }

                    else goto id;
                }
            }

            else
            {
                id:

                if (SEPARATORS.contains(buf))
                {
                    flush(WordType::Id, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }

                else
                {
                    while (getChar(input, buf) && (isalpha(buf) || isdigit(buf) || buf == '_'))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Id, out);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = ScanMode::Skip;
                    }
                }
            }
        }

        else if (buf == '_')
        {
            if (getChar(input, buf))
            {
                if (SEPARATORS.contains(buf))
                {
                    flush(WordType::Id, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;
                }

                else
                {
                    while (getChar(input, buf) && (isalpha(buf) || isdigit(buf) || buf == '_'))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Id, out);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = ScanMode::Skip;
                    }
                }
            }
        }

        else if (isdigit(buf))
        {
            if (getChar(input, buf))
            {
                if (SEPARATORS.contains(buf))
                {
                    flush(WordType::Literal, out);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = ScanMode::Skip;
                }

                else if (buf == '.')
                {
                    out << buf;

                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Literal, out);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = ScanMode::Skip;
                    }
                }

                else
                {
                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (buf == '.')
                        out << buf;

                    while (getChar(input, buf) && isdigit(buf))
                        out << buf;

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Literal, out);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = ScanMode::Skip;
                    }
                }
            }
        }

        else if (buf == '.')
        {
            while (getChar(input, buf) && isdigit(buf))
                out << buf;

            if (SEPARATORS.contains(buf))
            {
                flush(WordType::Literal, out);

                if (!TEXT_SEPARATORS.contains(buf))
                    out << buf;

                mode = ScanMode::Skip;
            }
        }

        else if (buf == '"')
        {
            while (getChar(input, buf) && buf != '"')
                out << buf;

            out << buf;
            flush(WordType::Literal, out);
        }
    }
}