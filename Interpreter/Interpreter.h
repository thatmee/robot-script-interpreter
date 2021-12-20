#pragma once
#include "LexAndParser/Parser.h"

class User;

class Interpreter
{
private:
    /// @brief ��װ�﷨��������﷨������
    Parser parser;

    /// @brief Ҫ�����Ľű��ļ�·����һ�� Parser ���Ӧһ���ű�
    const char* scriptPath;

    /// @brief ���� Out ������������
    void interpretOut(Out* out, User& user);

    /// @brief �������з�֧��ת���������� branch��listen��default
    void interpretBranch(StepActVec& actions, User& user);
public:
    /// @brief ������Ҫ���ظ��ⲿ�Ķ���״̬
    enum class STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    Interpreter(const char* scriptName_);

    ~Interpreter();

    /// @brief ��ʼ���﷨��
    /// @return 0 ʧ�� 1 �ɹ�
    bool initParseTree();

    /// @brief �����û���ִ�л���ִ�нű���������Ҫ���ظ��ⲿִ�еĶ�����ֹͣ�������û���ִ�л���
    /// @param user �����û���ִ�л���
    /// @return ��Ҫ�ⲿִ�еĶ���
    Interpreter::STA interpret(User& user);

    /// @brief �����﷨���ı�����Ϣ�����ú��û�������ļ�������ǰ��������Ϊ entry
    void initExecEnv(User& user);
};
