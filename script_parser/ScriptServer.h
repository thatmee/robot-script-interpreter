#pragma once
#include <iostream>
#include <fstream>
#include "ParseTree.h"
#include "Parser.h"

class ScriptServer
{
private:
    const char* scriptName;
    Parser parser;
public:
    ScriptServer(const char* scriptName_) : parser(scriptName_), scriptName(scriptName_) {}
    void parse();
};
