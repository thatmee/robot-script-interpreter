#pragma once
#include "LexAndParser/Parser.h"

class User;

class Interpreter
{
private:
    /// @brief 封装语法分析类和语法分析树
    Parser parser;

    /// @brief 要分析的脚本文件路径，一个 Parser 类对应一个脚本
    const char* scriptPath;

    /// @brief 解释 Out 动作，填充变量
    void interpretOut(Out* out, User& user);

    /// @brief 解释所有分支跳转动作，包括 branch、listen、default
    void interpretBranch(StepActVec& actions, User& user);
public:
    /// @brief 所有需要返回给外部的动作状态
    enum class STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    Interpreter(const char* scriptName_);

    ~Interpreter();

    /// @brief 初始化语法树
    /// @return 0 失败 1 成功
    bool initParseTree();

    /// @brief 根据用户的执行环境执行脚本，遇到需要返回给外部执行的动作则停止并更新用户的执行环境
    /// @param user 保存用户的执行环境
    /// @return 需要外部执行的动作
    Interpreter::STA interpret(User& user);

    /// @brief 根据语法树的变量信息，设置好用户变量表的键，将当前步骤设置为 entry
    void initExecEnv(User& user);
};
