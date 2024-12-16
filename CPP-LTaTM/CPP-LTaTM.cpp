#include "Parser.h"

#include "Tree.h"

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
    /*
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
    */

    Tree<int> tree(0);
    tree.push_back(1);
    tree.push_back(2);
    tree.push_back(3);
    tree.select(0);
    tree.emplace_back(3);
    tree.emplace_front(4);
    tree.emplace_front(5);
    tree.print();

    /*
    auto& tokens = lexer.tokenize();

    for (auto& token : tokens)
        cout << '\t' << token.value << "\t\t: " << Lexer::tokenTypeToString(token.type) << '\n';

    */

    return 0;
}