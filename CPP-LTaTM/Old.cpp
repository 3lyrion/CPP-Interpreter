#include "Precompiled.h"

enum class WordType
{
    Id = 0,
    Type,
    Operator,
    BoolLiteral,
    FloatLiteral,
    IntLiteral,
    StringLiteral,
    Separator,
    Keyword
};

class Lexer
{
public:
    inline static const unordered_set<char> SEPARATORS
    {
        '}', ')', '^', '-', '+', '*', '/', '%', '<', '>', '<', '>',
        '=', '&', '|', ';', ' ', '\n', '\t', '\r'
    };

    inline static const unordered_set<string> OPERATORS
    {
        "(", "{", "}", ")", "^", "-", "+", "*", "/", "%", "<", ">", "<=", ">=",
        "=", "==", "&&", "||"
    };

    inline static const unordered_set<char> TEXT_SEPARATORS
    {
        ' ', '\n', '\t', '\r'
    };

private:
    using Token = pair<WordType, string>;

    enum class Mode
    {
        Read = 0,
        Skip,
        Shutdown
    };

    bool scanning = false;

    char prevBuf = 0;
    char buf = 0;

    uint32_t strCount = 1;

    Mode mode = Mode::Read;

    ostringstream out;

    vector<Token> tokens;

    bool getChar(stringstream& input)
    {
        if (input.eof())
        {
            if (scanning) error("Непредвиденный конец файла");

            return false;
        }

        prevBuf = buf;

        input.get(buf);

        return true;
    }

    void flush(WordType type)
    {
        switch (type)
        {
        case WordType::Id: cout << "Идентификатор: ";
            break;
        case WordType::Type: cout << "Тип: ";
            break;
        case WordType::Operator: cout << "Оператор: ";
            break;
        case WordType::BoolLiteral: cout << "Литерал (bool): ";
            break;
        case WordType::FloatLiteral: cout << "Литерал (float): ";
            break;
        case WordType::IntLiteral: cout << "Литерал (int): ";
            break;
        case WordType::StringLiteral: cout << "Литерал (string): ";
            break;
        case WordType::Separator: cout << "Разделитель: ";
            break;
        case WordType::Keyword: cout << "Ключевое слово: ";
            break;
        default:
            break;
        }

        auto t = out.str();

        tokens.emplace_back(type, t);

        cout << t << '\n';
        out.str("");
    }

    void error(string const& descr = "Ошибка")
    {
        out << buf;

        cerr << descr << ", строка " << strCount << ": " << out.str() << '\n';

        throw "Error";
    }

    void scan(stringstream& input)
    {
        try
        {

        while (mode != Mode::Shutdown)
        {
            if (mode == Mode::Skip)
                mode = Mode::Read;

            else if (mode == Mode::Read)
            {
                if (!getChar(input)) break;

                if (TEXT_SEPARATORS.contains(buf))
                {
                    if (buf == '\n') strCount++;

                    continue;
                }

                out << buf;
            }

            scanning = true;

            if (buf == '{')
                flush(WordType::Separator);

            else if (buf == '}')
                flush(WordType::Separator);

            else if (buf == ';')
                flush(WordType::Separator);

            else if (buf == '(')
                flush(WordType::Operator);

            else if (buf == ')')
                flush(WordType::Operator);

            else if (buf == '+')
                flush(WordType::Operator);

            else if (buf == '-')
            {
                if (getChar(input))
                {
                    auto type = WordType::IntLiteral;

                    if (isdigit(buf))
                    {
                        out << buf;

                        while (getChar(input) && isdigit(buf))
                            out << buf;

                        if (getChar(input) && buf == '.')
                        {
                            out << buf;
                            type = WordType::FloatLiteral;
                        }

                        while (getChar(input) && isdigit(buf))
                            out << buf;

                        if (SEPARATORS.contains(buf))
                        {
                            flush(type);

                            out << buf;
                        }
                    }

                    else if (buf == '.')
                    {
                        type = WordType::FloatLiteral;

                        while (getChar(input) && isdigit(buf))
                            out << buf;

                        if (SEPARATORS.contains(buf))
                        {
                            flush(type);

                            out << buf;
                        }
                    }

                    else if (isalpha(buf) || TEXT_SEPARATORS.contains(buf))
                    {
                        flush(WordType::Operator);

                        out << buf;

                        mode = Mode::Skip;
                    }

                    else error();
                }
            }

            else if (buf == '*')
                flush(WordType::Operator);

            else if (buf == '/')
            {
                if (getChar(input))
                {
                    if (buf == '/')
                    {
                        /*
                        out << buf;

                        flush(WordType::Separator);
                        */

                        out.str("");

                        while (getChar(input) && buf != '\n');
                    }

                    else if (buf == '*')
                    {
                        /*
                        out << buf;

                        flush(WordType::Separator);
                        */

                        out.str("");

                        comment:

                        while (getChar(input) && buf != '*');

                        if (getChar(input) && buf == '/')
                        {
                            /*
                            out << '*' << buf;

                            flush(WordType::Separator);
                            */
                        }

                        else goto comment;
                    }

                    else
                    {
                        flush(WordType::Operator);

                        out << buf;

                        mode = Mode::Skip;
                    }
                }
            }

            else if (buf == '%')
                flush(WordType::Operator);

            else if (buf == '^')
                flush(WordType::Operator);

            else if (buf == '!')
            {
                if (getChar(input))
                {
                    if (buf == '=')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else
                    {
                        flush(WordType::Operator);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }
                }
            }

            else if (buf == '=')
            {
                if (getChar(input))
                {
                    if (buf == '=')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else
                    {
                        flush(WordType::Operator);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }
                }
            }

            else if (buf == '>')
            {
                if (getChar(input))
                {
                    if (buf == '=')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else
                    {
                        flush(WordType::Operator);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }
                }
            }

            else if (buf == '<')
            {
                if (getChar(input))
                {
                    if (buf == '=')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else
                    {
                        flush(WordType::Operator);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }
                }
            }

            else if (buf == '&')
            {
                if (getChar(input))
                {
                    if (buf == '&')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else error();
                }
            }

            else if (buf == '|')
            {
                if (getChar(input))
                {
                    if (buf == '|')
                    {
                        out << buf;

                        flush(WordType::Operator);
                    }

                    else error();
                }
            }

            else if (isalpha(buf))
            {
                if (buf == 'p')
                {
                    if (getChar(input))
                    {
                        out << buf;

                        if (buf == 'r')
                        {
                            if (getChar(input))
                            {
                                out << buf;

                                if (buf == 'i')
                                {
                                    if (getChar(input))
                                    {
                                        out << buf;

                                        if (buf == 'n')
                                        {
                                            if (getChar(input))
                                            {
                                                out << buf;

                                                if (buf == 't')
                                                {
                                                    if (getChar(input))
                                                    {
                                                        if (TEXT_SEPARATORS.contains(buf))
                                                            flush(WordType::Operator);

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
                    if (getChar(input))
                    {
                        out << buf;

                        if (buf == 'o')
                        {

                            if (getChar(input))
                            {

                                    out << buf;

                                if (buf == 'n')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;


                                        if (buf == 's')
                                        {

                                            if (getChar(input))
                                            {
                                                    out << buf;

                                                if (buf == 't')
                                                {

                                                    if (getChar(input))
                                                    {
                                                        if (TEXT_SEPARATORS.contains(buf))
                                                            flush(WordType::Keyword);

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
                    if (getChar(input))
                    {
                            out << buf;


                        if (buf == 'o')
                        {

                            if (getChar(input))
                            {
                                    out << buf;


                                if (buf == 'o')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;

                                        if (buf == 'l')
                                        {

                                            if (getChar(input))
                                            {
                                                if (TEXT_SEPARATORS.contains(buf))
                                                    flush(WordType::Type);

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
                    if (getChar(input))
                    {
                            out << buf;


                        if (buf == 'n')
                        {

                            if (getChar(input))
                            {
                                    out << buf;


                                if (buf == 't')
                                {

                                    if (getChar(input))
                                    {


                                        if (TEXT_SEPARATORS.contains(buf))
                                            flush(WordType::Type);

                                        else goto id;
                                    }
                                }

                                else goto id;
                            }
                        }

                        else if (buf == 'f')
                        {
                            if (getChar(input))
                            {
                                if (TEXT_SEPARATORS.contains(buf))
                                    flush(WordType::Operator);

                                else if (buf == '(')
                                {
                                    flush(WordType::Operator);

                                    out << buf;

                                    mode = Mode::Skip;
                                }

                                else goto id;
                            }
                        }

                        else goto id;
                    }
                }

                else if (buf == 'f')
                {
                    if (getChar(input))
                    {
                            out << buf;


                        if (buf == 'l')
                        {

                            if (getChar(input))
                            {
                                    out << buf;


                                if (buf == 'o')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;


                                        if (buf == 'a')
                                        {

                                            if (getChar(input))
                                            {
                                                    out << buf;


                                                if (buf == 't')
                                                {

                                                    if (getChar(input))
                                                    {
                                                        if (TEXT_SEPARATORS.contains(buf))
                                                            flush(WordType::Type);

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

                            if (getChar(input))
                            {
                                out << buf;

                                if (buf == 'l')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;


                                        if (buf == 's')
                                        {

                                            if (getChar(input))
                                            {
                                                    out << buf;


                                                if (buf == 'e')
                                                {

                                                    if (getChar(input))
                                                    {
                                                        if (TEXT_SEPARATORS.contains(buf))
                                                            flush(WordType::BoolLiteral);

                                                        else if (buf == ')' || buf == '}')
                                                        {
                                                            flush(WordType::BoolLiteral);

                                                            out << buf;

                                                            mode = Mode::Skip;
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
                    if (getChar(input))
                    {
                            out << buf;


                        if (buf == 't')
                        {

                            if (getChar(input))
                            {
                                    out << buf;


                                if (buf == 'r')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;


                                        if (buf == 'i')
                                        {

                                            if (getChar(input))
                                            {
                                                    out << buf;


                                                if (buf == 'n')
                                                {

                                                    if (getChar(input))
                                                    {
                                                            out << buf;


                                                        if (buf == 'g')
                                                        {

                                                            if (getChar(input))
                                                            {
                                                                if (TEXT_SEPARATORS.contains(buf))
                                                                    flush(WordType::Type);

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
                    if (getChar(input))
                    {
                            out << buf;


                        if (buf == 'l')
                        {

                            if (getChar(input))
                            {
                                    out << buf;


                                if (buf == 's')
                                {

                                    if (getChar(input))
                                    {
                                            out << buf;


                                        if (buf == 'e')
                                        {

                                            if (getChar(input))
                                            {
                                                if (TEXT_SEPARATORS.contains(buf))
                                                    flush(WordType::Operator);

                                                else if (buf == '(')
                                                {
                                                    flush(WordType::Operator);

                                                    out << buf;

                                                    mode = Mode::Skip;
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
                    if (getChar(input))
                    {
                        out << buf;

                        if (buf == 'h')
                        {
                        

                            if (getChar(input))
                            {
                                out << buf;

                                if (buf == 'i')
                                {
                                

                                    if (getChar(input))
                                    {
                                        out << buf;

                                        if (buf == 'l')
                                        {
                                        

                                            if (getChar(input))
                                            {
                                                out << buf;

                                                if (buf == 'e')
                                                {
                                                

                                                    if (getChar(input))
                                                    {
                                                        if (TEXT_SEPARATORS.contains(buf))
                                                            flush(WordType::BoolLiteral);

                                                        else if (buf == '(')
                                                        {
                                                            flush(WordType::BoolLiteral);

                                                            out << buf;

                                                            mode = Mode::Skip;
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
                    if (getChar(input))
                    {
                        out << buf;

                        if (buf == 'r')
                        {
                        

                            if (getChar(input))
                            {
                                out << buf;

                                if (buf == 'u')
                                {
                                

                                    if (getChar(input))
                                    {
                                        out << buf;

                                        if (buf == 'e')
                                        {
                                        

                                            if (getChar(input))
                                            {
                                                if (TEXT_SEPARATORS.contains(buf))
                                                    flush(WordType::BoolLiteral);

                                                else if (buf == ')' || buf == '}')
                                                {
                                                    flush(WordType::BoolLiteral);

                                                    out << buf;

                                                    mode = Mode::Skip;
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

                else
                {
                    id:

                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Id);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }

                    else
                    {
                        while (getChar(input) && (isalpha(buf) || isdigit(buf) || buf == '_'))
                            out << buf;

                        if (SEPARATORS.contains(buf))
                        {
                            flush(WordType::Id);

                            if (!TEXT_SEPARATORS.contains(buf))
                                out << buf;

                            mode = Mode::Skip;
                        }
                    }
                }
            }

            else if (buf == '_')
            {
                if (getChar(input))
                {
                    if (SEPARATORS.contains(buf))
                    {
                        flush(WordType::Id);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;
                    }

                    else
                    {
                        while (getChar(input) && (isalpha(buf) || isdigit(buf) || buf == '_'))
                            out << buf;

                        if (SEPARATORS.contains(buf))
                        {
                            flush(WordType::Id);

                            if (!TEXT_SEPARATORS.contains(buf))
                                out << buf;

                            mode = Mode::Skip;
                        }
                    }
                }
            }

            else if (isdigit(buf))
            {
                auto type = WordType::IntLiteral;

                if (getChar(input))
                {
                    if (SEPARATORS.contains(buf))
                    {
                        flush(type);

                        if (!TEXT_SEPARATORS.contains(buf))
                            out << buf;

                        mode = Mode::Skip;
                    }

                    else if (buf == '.')
                    {
                        out << buf;

                        goto point;
                    }

                    else
                    {
                        out << buf;

                        while (getChar(input) && isdigit(buf))
                            out << buf;

                        if (buf == '.')
                        {
                            out << buf;
                            type = WordType::FloatLiteral;
                        }

                        while (getChar(input) && isdigit(buf))
                            out << buf;

                        if (SEPARATORS.contains(buf))
                        {
                            flush(type);

                            if (!TEXT_SEPARATORS.contains(buf))
                                out << buf;

                            mode = Mode::Skip;
                        }

                        else error();
                    }
                }
            }

            else if (buf == '.')
            {
                point:

                if (isdigit(prevBuf)) error();

                if (getChar(input) && !isdigit(buf)) error();

                out << buf;

                while (getChar(input) && isdigit(buf))
                    out << buf;

                if (SEPARATORS.contains(buf))
                {
                    flush(WordType::FloatLiteral);

                    if (!TEXT_SEPARATORS.contains(buf))
                        out << buf;

                    mode = Mode::Skip;
                }

                else error();
            }

            else if (buf == '"')
            {
                while (getChar(input) && buf != '"')
                    out << buf;

                out << buf;
                flush(WordType::StringLiteral);
            }

            else if (!TEXT_SEPARATORS.contains(buf)) error();

            scanning = false;
        }

        } // try

        catch (...)
        {

        }
    }

public:
    Lexer() = default;

    void scan(ifstream const& file)
    {
        stringstream input;

        input << file.rdbuf();;

        scan(input);
    }

    void scan(string const& str)
    {
        stringstream input;

        input << str;

        scan(input);
    }
};