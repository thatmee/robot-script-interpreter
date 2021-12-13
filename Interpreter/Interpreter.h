#pragma once
#include "LexAndParser/Parser.h"
#include "User.h"

class Interpreter
{
private:
    Parser parser;

public:
    Interpreter();
    void interpret(User& user);

    /// @brief 根据语法树的变量信息，设置好用户变量表的键，将当前步骤设置为 entry
    void initExecEnv(User& user);
};
