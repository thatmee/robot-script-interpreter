#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "LexAndParser/ParseTree.h"
#include "LexAndParser/Parser.h"
#include "User.h"
#include "Interpreter.h"

using UserTable = std::unordered_map<UserID, std::unique_ptr<User>>;


class ScriptServer
{
private:
    enum class ERR_STA
    {
        WrongID,
    };
    Parser parser;
    Interpreter interpreter;
    UserTable users;
    bool ready;


    void initServer();
    void initParser();
    bool isValidFormat(UserID& id);
    void error(ScriptServer::ERR_STA err);
    void anaInputMsg();
    void doAction();
    void updateUserData();

public:
    ScriptServer(const char* scriptName_) : parser(scriptName_), interpreter() {}

    void createUser(UserID ID_, UserName name_ = "");
};
