#pragma once
#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <regex>

class Action;

using StepID = std::string;
using VarName = std::string;
using KeyWord = std::string;
using StepActVec = std::vector<std::unique_ptr<Action>>;
using StepTable = std::unordered_map<StepID, StepActVec>;
using Item = std::string;
using Expression = std::list<Item>;

class ParseTree
{
public:
    StepID entry;
    std::vector<StepID> exits;
    std::list<KeyWord> keyWords;
    std::list<VarName> vars;
    StepTable stepTable;
    ParseTree() {}
    ~ParseTree();
    //void output();
};
