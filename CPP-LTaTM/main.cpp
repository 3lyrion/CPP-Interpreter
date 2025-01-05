#include "Precompiled.h"

#include "Parser.h"

//#include "Tree.h"

int main()
{
//    setlocale(LC_CTYPE, "Russian");

    locale::global(locale(".UTF-8"));

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

    setvbuf(stdout, nullptr, _IONBF, 0);
    
    Lexer lexer("input.txt");
    Parser parser(lexer);
    parser.parse();
    
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
    
    //Tree<int> tree1(0);
    //tree1.push_back(1);
    //tree1.push_back(2);
    ///*
    //tree1.push_back(3);
    //tree1.front();
    //tree1.push_back(4);
    //tree1.push_back(5);
    //tree1.push_back(6);
    //*/
    //Tree<int> tree2(3);
    //tree2.emplace_back(4);
    //tree2.emplace_front(5);
    //tree2.emplace_front(6);

    //tree1.push_back(move(tree2));

    //tree1.exclude({ 3 });

    //tree1.print();
    
    /*
    auto& tokens = lexer.tokenize();

    for (auto& token : tokens)
        cout << '\t' << token.value << "\t\t: " << Lexer::tokenTypeToString(token.type) << '\n';

    */

    return 0;
}