#pragma once
#include "LexAndParser/Parser.h"

class User;

class Interpreter
{
private:
    Parser parser;

    void interpretOut(Out* out, User& user);
    void interpretBranch(StepActVec& actions, User& user);
public:
    enum class STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    Interpreter(const char* scriptName_);
    ~Interpreter();
    void initParseTree();

    Interpreter::STA interpret(User& user);

    /// @brief 根据语法树的变量信息，设置好用户变量表的键，将当前步骤设置为 entry
    void initExecEnv(User& user);
};
