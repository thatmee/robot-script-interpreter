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
    enum class NEW_USER_STA
    {
        WrongIDFormat,
        AlreadyExists,
        Succeed
    };

    enum class DEL_USER_STA
    {
        NoSuchUser,
        Succeed
    };

    enum class INTERPRET_STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    ScriptServer(const char* scriptName_);
    ~ScriptServer();
    ScriptServer::NEW_USER_STA createUser(UserID ID_);
    ScriptServer::DEL_USER_STA deleteUser(UserID& ID_);
    void msgToUserInputKey(UserID& ID_, std::string msg);
    ScriptServer::INTERPRET_STA srvInterpret(UserID& ID_);
    void getOutputMsg(UserID& ID_, std::string& outputMsg);
    void setFinished(UserID& ID_);
private:
    Interpreter interpreter;
    UserTable users;
    bool isValidFormat(UserID& id);
    void error(ScriptServer::NEW_USER_STA err);
};
