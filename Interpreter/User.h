#pragma once
#include <string>
#include <unordered_map>
#include "LexAndParser/ParseTree.h"

using UserID = std::string;
using UserName = std::string;
using VarTable = std::unordered_map<VarName, std::string>;


class User
{
public:
    UserID ID = "";
    UserName name = "";
    StepID curStepID = "";
    VarTable vars;
    bool connected = false;

    User(UserID ID_, UserName name_ = "") : ID(ID_), name(name_) {}
};
