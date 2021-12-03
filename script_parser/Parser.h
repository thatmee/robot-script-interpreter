#pragma once
#include <string>
#include <vector>
#include "ParseTree.h"

using Line = std::string;
using Token = std::string;
using TokenStream = std::vector<Token>;



class Parser
{
private:
    enum class ERR_STATE
    {
        UnknownToken
    };
    TokenStream tokenStream;
    TokenStream::iterator cur;
    void trim(std::string&);
    void parseFile();

    void parseLine(Line);
    void procTokens();
    void procStep();
    void procOut();
    void procListen();
    void procBranch();
    void procSilence();
    void procDefault();
    void procExit();
    void error(ERR_STATE);



protected:
    const char* scriptName;
    ParseTree parseTree;

public:
    Parser() {}
    Parser(const char* scriptName_) :scriptName(scriptName_), parseTree() {}
    void generateParseTree();
};
