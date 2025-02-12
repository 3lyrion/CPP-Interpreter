#include <Parser.h>

int main()
{
    locale::global(locale("en_US.UTF-8"));

    setvbuf(stdout, nullptr, _IONBF, 0);
    
    Lexer lexer;
    auto& tokens = lexer.tokenize("input_1.txt");

    Parser parser;
    auto tree = parser.parse(tokens);

    Shell shell;
    shell.interpet(*tree);

    return EXIT_SUCCESS;
}