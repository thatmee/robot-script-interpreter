#pragma once
#include <unordered_map>
#include <list>
#include <vector>
#include "Action.h"

using StepID = int;
using Step = std::string;
using SymbolID = int;
using Symbol = std::string;
using VarName = std::string;
using KeyWord = std::string;
using StepActVec = std::vector<Action&>;
using Item = std::string;
using Expression = std::list<Item>;

class ParseTree
{
private:
    std::unordered_map<StepID, Step> stepTable;
    StepID entry;
    std::list<KeyWord> keyWords;
    //std::list<VarName> vars;
    std::unordered_map<SymbolID, Symbol> symbolTable;

    std::unordered_map<StepID, StepActVec> actionTable;

};
