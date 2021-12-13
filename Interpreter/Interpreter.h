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

    /// @brief �����﷨���ı�����Ϣ�����ú��û�������ļ�������ǰ��������Ϊ entry
    void initExecEnv(User& user);
};
