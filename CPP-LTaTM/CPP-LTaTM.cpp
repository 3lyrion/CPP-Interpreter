#include "Lexer.hpp"

int main()
{
    setlocale(LC_CTYPE, "Russian");

//    Lexer lexer;

    ifstream     file("input.txt", ios::in);
    stringstream input/*(R"(

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
    auto tokens = lexer.tokenize();

    

//    lexer.scan(file);
    
 //   file.close();

    return 0;
}