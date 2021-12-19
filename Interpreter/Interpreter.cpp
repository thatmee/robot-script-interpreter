#include "Interpreter.h"
#include "LexAndParser/Action.hpp"
#include "User.h"

Interpreter::Interpreter(const char* scriptName_) : parser(scriptName_)
{
    std::cout << "init interpreter..." << std::endl;
}

Interpreter::~Interpreter()
{

}

void Interpreter::initParseTree()
{
    std::cout << "init parser tree..." << std::endl;
    parser.generateParseTree();
}

void Interpreter::initExecEnv(User& user)
{
    // 填充用户变量表的键
    for (std::list<VarName>::iterator kIter = parser.parseTree.vars.begin(); kIter != parser.parseTree.vars.end(); kIter++)
    {
        user.vars.insert(VarTablePair(*kIter, ""));
    }

    // 根据 entry ID 填充用户的起始步骤 ID
    user.curStepID = parser.parseTree.entry;
}

Interpreter::STA Interpreter::interpret(User& user)
{
    // 上一个动作没有完成，继续完成
    if (!user.finished)
    {
        return Interpreter::STA::Do;
    }

    // 上一个动作已完成，将动作指针 +1，完成标志置为未完成
    user.actIndex++;
    user.finished = false;
    user.outputMsg = "";
    user.listenTime = -1;

    // 循环，根据语法树确定下一步的跳转或者动作
    while (user.actIndex < parser.parseTree.stepTable[user.curStepID].size())
    {
        // 获取当前步骤下的动作队列
        StepActVec& actions = parser.parseTree.stepTable[user.curStepID];

        // 获取新的动作类型
        Action::ActionType curActType = actions[user.actIndex]->getCurType();

        // 根据动作类型，综合用户信息，得到服务器要执行的动作
        if (curActType == Action::ActionType::Out)
        {
            // 将基类 Action 转化为派生类 Out，获取输出字符串
            Out* out = static_cast<Out*>(actions[user.actIndex].get());
            interpretOut(out, user);
            // 直接返回
            return Interpreter::STA::Out;
        }
        else if (curActType == Action::ActionType::Listen)
        {
            // 获取要 listen 的时间
            Listen* listen = static_cast<Listen*>(actions[user.actIndex].get());
            user.listenTime = listen->getListenTime();
            // 直接返回
            return Interpreter::STA::Listen;
        }
        else if (curActType == Action::ActionType::Branch)
        {
            interpretBranch(actions, user);
        }
        else if (curActType == Action::ActionType::Exit)
        {
            // 直接返回
            return Interpreter::STA::Exit;
        }

        user.actIndex++;
    }

    // 动作指针超出了当前步骤的动作数，退出
    return Interpreter::STA::Exit;
}


void Interpreter::interpretOut(Out* out, User& user)
{
    Expression outInfo;
    std::string outStr;

    // 获取要输出的信息，填充变量
    out->getOutInfo(outInfo);
    for (Expression::iterator iter = outInfo.begin(); iter != outInfo.end(); iter++)
    {
        if (iter->at(0) == '$')
            outStr += user.vars[*iter];
        else
            outStr += *iter;
    }

    user.outputMsg = outStr;
}

void Interpreter::interpretBranch(StepActVec& actions, User& user)
{
    int silenceIndex = -1, defaultIndex = -1;
    for (int i = user.actIndex; i < actions.size(); i++)
    {
        // 用户输入为空，且当前为 silence 分支
        if (user.inputKey == "" && actions[i]->getCurType() == Action::ActionType::Silence)
        {
            silenceIndex = i;
            Silence* silence = static_cast<Silence*>(actions[i].get());
            StepID nextStepID;
            silence->getNextStepID(nextStepID);
            user.curStepID = nextStepID;
            user.actIndex = -1;
            break;
        }
        // 用户有输入，且当前动作是有关键词的分支
        else if (user.inputKey != "" && actions[i]->getCurType() == Action::ActionType::Branch)
        {
            Branch* branch = static_cast<Branch*>(actions[i].get());
            KeyWord expectKey;
            branch->getKeyWord(expectKey);
            // 没有匹配到关键词，继续查看下一个动作
            if (user.inputKey != expectKey)
                continue;
            // 匹配到关键字，转到对应的 step 和 actionIndex
            StepID nextStepID;
            branch->getNextStepID(nextStepID);
            user.curStepID = nextStepID;
            user.actIndex = -1;
            break;
        }
        // 用户有输入，且当前为 default 分支
        else if (user.inputKey != "" && actions[i]->getCurType() == Action::ActionType::Default)
        {
            defaultIndex = i;
            Default* d = static_cast<Default*>(actions[i].get());
            StepID nextStepID;
            d->getNextStepID(nextStepID);
            user.curStepID = nextStepID;
            user.actIndex = -1;
            break;
        }
    }
}