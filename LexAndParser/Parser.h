#pragma once
#define GTEST
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <lib\magic_enum.hpp>
#include <codecvt>
#include "ParseTree.h"
#include "LexicalAnalyzer.h"
#include "Action.hpp"

using Line = std::string;
using Token = std::string;
using TokenStream = std::vector<Token>;

class Parser
{
private:
    enum class ERR_STATE
    {
        UnknownToken,
        DuplicatedStep,
        TooManyTokens,
        TooFewTokens,
        LexicalError,
        WrongExprssion,
        NoCorrespondingStep,
        WrongType
    };
    enum class EXPR_STATE
    {
        WaitAdd,
        WaitToken
    };

    int lineCnt;
    TokenStream tokenStream;
    StepID curStepID = "";
    Token popFront(TokenStream&);
    void trim(std::string&);
    void splitString(const std::string&, std::vector<std::string>&, const std::string&);
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
    void error(ERR_STATE err);


protected:
    const char* scriptPath;
    ParseTree parseTree;
#ifdef GTEST
    std::ofstream logFile;
#endif // GTEST


public:
    Parser() : scriptPath(nullptr), lineCnt(1) {}
    Parser(const char* scriptPath_);
    void generateParseTree();
};
