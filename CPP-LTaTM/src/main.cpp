#include <Parser.h>

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
    
    Lexer lexer;
    auto& tokens = lexer.tokenize("input.txt");

    Parser parser;
    auto tree = parser.parse(tokens);

    Shell shell;
    shell.interpet(*tree.release());
    
 //   list<int> l1 { 1, 2, 5 };
 //   list<int> l2 { 2, 3, 4 };

	//auto it = find(l1.begin(), l1.end(), 2);
	//it = l1.erase(it);
	//l1.insert(it, l2.begin(), l2.end());
 //   
 //   for (auto it = l.rbegin(); it != l.rend(); )
	//{
	//	auto ch = *it;

	//		it = (decltype(it))(l.erase(--it.base()));
	//}

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