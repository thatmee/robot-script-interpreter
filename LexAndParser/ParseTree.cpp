#include "ParseTree.h"
#include "Action.hpp"

ParseTree::~ParseTree() { }

void ParseTree::output(std::string logName)
{
    DBG dbg;

#ifdef GTEST
    dbg.setDbgLevel(DBG::DBG_LEVELS::Test);
    dbg.setOutPipe(DBG::PIPES::FileIO);
    DBG::DBG_LEVELS level = DBG::DBG_LEVELS::Test;

#else
    dbg.setDbgLevel(DBG::DBG_LEVELS::Detail);
    DBG::DBG_LEVELS level = DBG::DBG_LEVELS::Simple;
#endif // GTEST

    // 入口
    dbg.out("entry: " + entry + "\n", level, logName);

    // 出口
    dbg.out("exits: \n", level, logName);
    for (std::vector<StepID>::iterator iter = exits.begin(); iter != exits.end(); iter++)
        dbg.out("\t" + *iter + "\n", level, logName);

    // 关键词
    dbg.out("key words: \n", level, logName);
    for (std::list<KeyWord>::iterator iter = keyWords.begin(); iter != keyWords.end(); iter++)
        dbg.out("\t" + *iter + "\n", level, logName);

    // 变量
    dbg.out("vars: \n", level, logName);
    for (std::list<VarName>::iterator iter = vars.begin(); iter != vars.end(); iter++)
        dbg.out("\t" + *iter + "\n", level, logName);

    // 步骤表
    dbg.out("step table: \n", level, logName);
    // 遍历步骤表
    for (StepTable::iterator tIter = stepTable.begin(); tIter != stepTable.end(); tIter++)
    {
        dbg.out(tIter->first + "\n", level, logName);
        for (StepActVec::iterator vIter = tIter->second.begin(); vIter != tIter->second.end(); vIter++)
        {
            Action::ActionType type = (*vIter)->getCurType();
            StepID nextStepID;

            dbg.out("\t-> " + std::string(magic_enum::enum_name(type)) + "\t-> ", level, logName);

            if (type == Action::ActionType::Branch)
            {
                Branch* branch = static_cast<Branch*>(vIter->get());
                branch->getNextStepID(nextStepID);
                KeyWord key;
                branch->getKeyWord(key);
                dbg.out("\tkey: " + key + "\tnextStepID: " + nextStepID + "\n", level, logName);
            }
            else if (type == Action::ActionType::Silence)
            {
                Silence* silence = static_cast<Silence*>(vIter->get());
                silence->getNextStepID(nextStepID);
                dbg.out("\tnextStepID: " + nextStepID + "\n", level, logName);
            }
            else if (type == Action::ActionType::Default)
            {
                Default* d = static_cast<Default*>(vIter->get());
                d->getNextStepID(nextStepID);
                dbg.out("\tnextStepID: " + nextStepID + "\n", level, logName);
            }
            else if (type == Action::ActionType::Out)
            {
                Out* out = static_cast<Out*>(vIter->get());
                Expression outInfo;
                std::string outStr;
                out->getOutInfo(outInfo);
                dbg.out("\t", level, logName);
                for (Expression::iterator iter = outInfo.begin(); iter != outInfo.end(); iter++)
                    dbg.out(*iter, level, logName);
                dbg.out("\n", level, logName);
            }
            else if (type == Action::ActionType::Listen)
            {
                Listen* listen = static_cast<Listen*>(vIter->get());
                std::string listenTime = std::to_string(listen->getListenTime());
                dbg.out("\tlistenTime: " + listenTime + "\n", level, logName);
            }
        }
    }
}