#pragma once
#define GTEST
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <lib\magic_enum.hpp>
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
        WrongType
    };
    enum class EXPR_STATE
    {
        WaitAdd,
        WaitToken
    };

    int lineCnt;
    TokenStream tokenStream;
    StepID curStepID;
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
    void error(ERR_STATE err) {
#ifdef GTEST
        logFile << "line " << lineCnt << "\terror:" << magic_enum::enum_name(err) << std::endl;

#else
        // ´¦Àí´íÎó
        std::cout << "error:" << magic_enum::enum_name(err) << std::endl;
#endif // GTEST
    }



protected:
    const char* scriptName;
    ParseTree parseTree;
#ifdef GTEST
    std::ofstream logFile;
#endif // GTEST


public:
    Parser() : scriptName(nullptr), lineCnt(1) {}
    Parser(const char* scriptName_) :scriptName(scriptName_), parseTree(), lineCnt(1) {
#ifdef GTEST
        logFile.open("./log/log.txt");
        if (!logFile.is_open())
            std::cout << "fail to open log file." << std::endl;
#endif // GTEST
    }
    void generateParseTree() {
        std::cout << "generateParseTree" << std::endl;
    }
};
