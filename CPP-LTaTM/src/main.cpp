#include <Parser.h>

int main()
{
    locale::global(locale("en_US.UTF-8"));

    setvbuf(stdout, nullptr, _IONBF, 0);
    
    Lexer lexer;
    auto& tokens = lexer.tokenize("input_2.txt");

    Parser parser;
    auto tree = parser.parse(tokens);

    Shell shell;
    shell.interpet(*tree);

    //bool finish = false;
    //float sum = 5.0f;
    //const float FACTOR = 2.0f;
    //string str = "Abcdef";
    //int count;
    //count = 0;
    //int d = 5;

    //if (d % 2 == 0)
    //{
    //}
    //else
    //{
    //    sum=0.0;
    //}

    ///*
    //Цикл
    //*/
    //while (finish == false)
    //{
    //    sum = 5.0f+(sum+10.0f) * -powf(FACTOR, 1.50f);
    //    count = count + 1;
    //    finish = !(count > 100 || sum <= 1.0 && sum >= 0.0);	
    //}

    //cout << "Result\n";
    //cout << sum << '\n';

    return EXIT_SUCCESS;
}