#include "Interpreter.h"

Interpreter::Interpreter(const char* scriptName_) : parser(scriptName_)
{
    std::cout << "init interpreter..." << std::endl;
    std::cout << "init parser tree..." << std::endl;
    parser.generateParseTree();
    std::cout << "done: init parser tree." << std::endl;
    std::cout << "done: init interpreter." << std::endl;

}