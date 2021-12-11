#pragma once
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
        // ¥¶¿Ì¥ÌŒÛ
        std::cout << "error:" << magic_enum::enum_name(err) << std::endl;
    }



protected:
    const char* scriptName;
    ParseTree parseTree;

public:
    Parser() : scriptName(nullptr) {}
    Parser(const char* scriptName_) :scriptName(scriptName_), parseTree() {}
    void generateParseTree() {
        std::cout << "generateParseTree" << std::endl;
    }
};
