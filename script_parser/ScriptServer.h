#pragma once
#include <iostream>
#include <fstream>
#include "ParseTree.h"
#include "Parser.h"

class ScriptServer
{
public:
    ParseTree parseTree;
    const char* scriptName;
    Parser parser;
    ScriptServer(const char* scriptName_);
};
