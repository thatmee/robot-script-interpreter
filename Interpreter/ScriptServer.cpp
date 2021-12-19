#include "ScriptServer.h"
#include "LexAndParser/Action.hpp"
#include "User.h"

ScriptServer::ScriptServer(const char* scriptName_) : interpreter(scriptName_)
{
    interpreter.initParseTree();
}

ScriptServer::~ScriptServer() {}

ScriptServer::NEW_USER_STA ScriptServer::createUser(UserID ID_) {
    // �ж� ID �Ƿ���Ϲ��򣬲���������������ģ��
    if (!isValidFormat(ID_))
    {
        error(ScriptServer::NEW_USER_STA::WrongIDFormat);
        return ScriptServer::NEW_USER_STA::WrongIDFormat;
    }

    // �ж� ID �Ƿ��Ѵ��ڣ��Ѵ�������������ģ��
    if (users.find(ID_) != users.end())
    {
        error(ScriptServer::NEW_USER_STA::AlreadyExists);
        return ScriptServer::NEW_USER_STA::AlreadyExists;
    }

    // ���� userTable
    users.insert(UserPair(ID_, std::make_unique<User>(ID_)));
    User& user = static_cast<User&>(*users[ID_]);

    // ���� Interpreter������ parser tree ��ʼ���û�����
    interpreter.initExecEnv(user);

    // ʹ�������У�����û��ı�����
    for (VarTable::iterator iter = user.vars.begin(); iter != user.vars.end(); iter++)
    {
        std::cout << "������ " << iter->first << " : ";
        std::string answer;
        std::getline(std::cin, answer);
    }
    return ScriptServer::NEW_USER_STA::Succeed;
}

ScriptServer::DEL_USER_STA ScriptServer::deleteUser(UserID& ID_)
{
    UserTable::iterator iter = users.find(ID_);
    if (iter == users.end())
        return ScriptServer::DEL_USER_STA::NoSuchUser;
    users.erase(iter);
    return ScriptServer::DEL_USER_STA::Succeed;
}


ScriptServer::INTERPRET_STA ScriptServer::srvInterpret(UserID& ID_)
{
    User& user = static_cast<User&>(*users[ID_]);
    Interpreter::STA result = interpreter.interpret(user);
    switch (result)
    {
    case Interpreter::STA::Do:
        return ScriptServer::INTERPRET_STA::Do;
    case Interpreter::STA::Listen:
        return ScriptServer::INTERPRET_STA::Listen;
    case Interpreter::STA::Exit:
        return ScriptServer::INTERPRET_STA::Exit;
    case Interpreter::STA::Out:
        return ScriptServer::INTERPRET_STA::Out;
        // no default case
    }
}


bool ScriptServer::isValidFormat(UserID& id)
{
    if (id.length() != 3 || id[0] == '0')
        return false;
    try
    {
        int tmp = std::stoi(id);
    }
    catch (std::invalid_argument&)
    {
        return false;
    }
    return true;
}


void ScriptServer::error(ScriptServer::NEW_USER_STA err)
{
    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}


void ScriptServer::msgToUserInputKey(UserID& ID_, std::string msg)
{
    // ���������Ϣת��Ϊ�ؼ��ֱ��浽�û�����
    users[ID_]->inputKey = msg;
}

void ScriptServer::getOutputMsg(UserID& ID_, std::string& outputMsg)
{
    outputMsg = users[ID_]->outputMsg;
}


