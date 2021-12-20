#include "ScriptServer.h"
#include "LexAndParser/Action.hpp"
#include "User.h"

const int FILE_BUFFER_SIZE = 50;

ScriptServer::ScriptServer(const char* scriptName_) : interpreter(scriptName_), scriptPath(scriptName_)
{
    dbg.anaLogName(scriptName_, logName);
}


bool ScriptServer::init()
{
#ifdef GTEST

    dbg.setDbgLevel(DBG::DBG_LEVELS::Detail);
    dbg.setOutPipe(DBG::PIPES::FileIO);

#else
    dbg.setDbgLevel(DBG::DBG_LEVELS::Simple);
    dbg.setOutPipe(DBG::PIPES::Standard);

#endif // GTEST

    return interpreter.initParseTree();
}


ScriptServer::~ScriptServer() {}


ScriptServer::NEW_USER_STA ScriptServer::createUser(UserID ID_) {
    // 判断 ID 是否符合规则，不符合则进入错误处理模块
    if (!isValidFormat(ID_))
    {
        error(ScriptServer::NEW_USER_STA::WrongIDFormat);
        return ScriptServer::NEW_USER_STA::WrongIDFormat;
    }

    // 判断 ID 是否已存在，已存在则进入错误处理模块
    if (users.find(ID_) != users.end())
    {
        error(ScriptServer::NEW_USER_STA::AlreadyExists);
        return ScriptServer::NEW_USER_STA::AlreadyExists;
    }

    // 加入 userTable
    users.insert(UserPair(ID_, std::make_unique<User>(ID_)));
    User& user = static_cast<User&>(*users[ID_]);

    // 调用 Interpreter，根据 parser tree 初始化用户数据
    interpreter.initExecEnv(user);

#ifdef GTEST

    std::string inFileName;
    dbg.anaInfileName(scriptPath, inFileName);
    inFile.open(inFileName);
    if (!inFile.is_open())
    {
        dbg.out("fail to open input file.");
        exit(0);
    }

#endif // GTEST


    // 填充用户的变量表，使用输入模拟服务器数据库
    for (VarTable::iterator iter = user.vars.begin(); iter != user.vars.end(); iter++)
    {
        dbg.out("请输入 " + iter->first + " : ", logName);
        std::string answer;
        dbg.getline(inFile, FILE_BUFFER_SIZE, answer);

        iter->second = answer;
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
    // 规定 id 为 3 位，且第一位不为 0 的数字
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
    dbg.out("error: " + std::string(magic_enum::enum_name(err)) + "\n", logName);
}


void ScriptServer::msgToUserInputKey(UserID& ID_, std::string msg)
{
    // 将输入的信息转换为关键字保存到用户的类
    users[ID_]->inputKey = msg;
}


void ScriptServer::getOutputMsg(UserID& ID_, std::string& outputMsg)
{
    outputMsg = users[ID_]->outputMsg;
}


void ScriptServer::setFinished(UserID& ID_)
{
    users[ID_]->finished = true;
}


