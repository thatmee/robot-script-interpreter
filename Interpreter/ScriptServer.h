#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Interpreter.h"


using UserID = std::string;
using UserTable = std::unordered_map<UserID, std::unique_ptr<User>>;
using UserPair = std::pair<UserID, std::unique_ptr<User>>;

class ScriptServer
{
public:
    /// @brief �������û����״̬
    enum class NEW_USER_STA
    {
        WrongIDFormat,
        AlreadyExists,
        Succeed
    };

    /// @brief ɾ���û����״̬
    enum class DEL_USER_STA
    {
        NoSuchUser,
        Succeed
    };

    /// @brief ��������Ҫ�ⲿִ�еĶ���״̬
    enum class INTERPRET_STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    /// @brief ���ڲ��Ե������ļ�
    std::ifstream inFile;

    /// @brief ���ڲ��Ե�����ļ�
    std::string logName;

    ScriptServer(const char* scriptName_);

    ~ScriptServer();

    /// @brief ��ʼ���ű�������
    bool init();

    /// @brief �����û� id �������û�
    ScriptServer::NEW_USER_STA createUser(UserID ID_);

    /// @brief �����û� id ɾ���û�
    ScriptServer::DEL_USER_STA deleteUser(UserID& ID_);

    /// @brief ��Ȼ���Դ���ģ��ӿ�
    void msgToUserInputKey(UserID& ID_, std::string msg);

    /// @brief �û� ID_ ִ�нű�
    ScriptServer::INTERPRET_STA srvInterpret(UserID& ID_);

    /// @brief ��ȡҪ���͸��û�����Ϣ
    void getOutputMsg(UserID& ID_, std::string& outputMsg);

    /// @brief ������Ҫ�ⲿִ�еĶ��������
    void setFinished(UserID& ID_);
private:

    /// @brief ���������װ
    DBG dbg;

    /// @brief �ű�·��
    const char* scriptPath;

    /// @brief ��װ�ű�������
    Interpreter interpreter;

    /// @brief �����û�
    UserTable users;

    /// @brief �ж��û� ID �Ƿ���Ч
    bool isValidFormat(UserID& id);

    /// @brief ������ģ��
    void error(ScriptServer::NEW_USER_STA err);
};
