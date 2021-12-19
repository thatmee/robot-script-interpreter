#pragma once
#include <string>
#include <unordered_map>
#include "LexAndParser/ParseTree.h"

using UserID = std::string;
using UserName = std::string;
using VarTable = std::unordered_map<VarName, std::string>;
using VarTablePair = std::pair<VarName, std::string>;

class User
{
public:
    UserID ID = "";
    UserName name = "";
    StepID curStepID = "";
    int actIndex = -1;
    VarTable vars;
    std::string inputKey = "";

    bool finished = true;
    std::string outputMsg = "";
    int listenTime = -1;


    User(UserID ID_, UserName name_ = "") : ID(ID_), name(name_) {}
    ~User();
};
