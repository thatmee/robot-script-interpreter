#pragma once
#include <unordered_map>
#include <list>
#include <vector>
#include "Action.h"

using StepID = std::string;
using VarName = std::string;
using KeyWord = std::string;
using StepActVec = std::vector<Action*>;
using Item = std::string;
using Expression = std::list<Item>;

class ParseTree
{
public:
    StepID entry;
    std::list<KeyWord> keyWords;
    std::list<VarName> vars;
    std::unordered_map<StepID, StepActVec> stepTable;
    ParseTree();
    ~ParseTree();
    void output();
};
