#include "Parser.h"

int main()
{
    setlocale(LC_CTYPE, "Russian");

    /*stringstream input(R"(

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

        
        // Цикл
        while (true)
        {
            ;
        }

        print "Рус";            

    )")*/;

    Lexer lexer("input.txt");
    auto result = lexer.tokenize();

    if (!result.success) return EXIT_FAILURE;

    Parser parser;
    parser.parse(result.tokens);

    /*
    auto& tokens = lexer.tokenize();

    for (auto& token : tokens)
        cout << '\t' << token.value << "\t\t: " << Lexer::tokenTypeToString(token.type) << '\n';

    */

    return 0;
}