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

    /// @brief �����﷨���ı�����Ϣ�����ú��û�������ļ�������ǰ��������Ϊ entry
    void initExecEnv(User& user);
};
