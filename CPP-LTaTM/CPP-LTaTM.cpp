#include "Parser.h"

#include "BinTree.h"

int main()
{
//    setlocale(LC_CTYPE, "Russian");

    locale::global(locale("ru_RU.UTF-8"));

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

    /*Lexer lexer("input.txt");
    Parser parser(lexer);
    parser.parse();*/

    /*auto lexer = make_unique<Lexer>("input.txt");
    auto parser = make_unique<Parser>(*lexer);
    parser->parse();*/

    BinTree<int> btree(0);

    btree.push_left(1);
    btree.push_right(2);
    btree.left();
    btree.emplace_left(3);
    btree.emplace_right(4);
    btree.back();
    btree.right();
    btree.push_left(5);
    btree.begin();

    btree.bypass();

    

    /*
    auto& tokens = lexer.tokenize();

    for (auto& token : tokens)
        cout << '\t' << token.value << "\t\t: " << Lexer::tokenTypeToString(token.type) << '\n';

    */

    return 0;
}