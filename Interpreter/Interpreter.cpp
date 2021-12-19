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
    // ����û�������ļ�
    for (std::list<VarName>::iterator kIter = parser.parseTree.vars.begin(); kIter != parser.parseTree.vars.end(); kIter++)
    {
        user.vars.insert(VarTablePair(*kIter, ""));
    }

    // ���� entry ID ����û�����ʼ���� ID
    user.curStepID = parser.parseTree.entry;
}

Interpreter::STA Interpreter::interpret(User& user)
{
    // ��һ������û����ɣ��������
    if (!user.finished)
    {
        return Interpreter::STA::Do;
    }

    // ��һ����������ɣ�������ָ�� +1����ɱ�־��Ϊδ���
    user.actIndex++;
    user.finished = false;
    user.outputMsg = "";
    user.listenTime = -1;

    // ѭ���������﷨��ȷ����һ������ת���߶���
    while (user.actIndex < parser.parseTree.stepTable[user.curStepID].size())
    {
        // ��ȡ��ǰ�����µĶ�������
        StepActVec& actions = parser.parseTree.stepTable[user.curStepID];

        // ��ȡ�µĶ�������
        Action::ActionType curActType = actions[user.actIndex]->getCurType();

        // ���ݶ������ͣ��ۺ��û���Ϣ���õ�������Ҫִ�еĶ���
        if (curActType == Action::ActionType::Out)
        {
            // ������ Action ת��Ϊ������ Out����ȡ����ַ���
            Out* out = static_cast<Out*>(actions[user.actIndex].get());
            interpretOut(out, user);
            // ֱ�ӷ���
            return Interpreter::STA::Out;
        }
        else if (curActType == Action::ActionType::Listen)
        {
            // ��ȡҪ listen ��ʱ��
            Listen* listen = static_cast<Listen*>(actions[user.actIndex].get());
            user.listenTime = listen->getListenTime();
            // ֱ�ӷ���
            return Interpreter::STA::Listen;
        }
        else if (curActType == Action::ActionType::Branch)
        {
            interpretBranch(actions, user);
        }
        else if (curActType == Action::ActionType::Exit)
        {
            // ֱ�ӷ���
            return Interpreter::STA::Exit;
        }

        user.actIndex++;
    }

    // ����ָ�볬���˵�ǰ����Ķ��������˳�
    return Interpreter::STA::Exit;
}


void Interpreter::interpretOut(Out* out, User& user)
{
    Expression outInfo;
    std::string outStr;

    // ��ȡҪ�������Ϣ��������
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
        // �û�����Ϊ�գ��ҵ�ǰΪ silence ��֧
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
        // �û������룬�ҵ�ǰ�������йؼ��ʵķ�֧
        else if (user.inputKey != "" && actions[i]->getCurType() == Action::ActionType::Branch)
        {
            Branch* branch = static_cast<Branch*>(actions[i].get());
            KeyWord expectKey;
            branch->getKeyWord(expectKey);
            // û��ƥ�䵽�ؼ��ʣ������鿴��һ������
            if (user.inputKey != expectKey)
                continue;
            // ƥ�䵽�ؼ��֣�ת����Ӧ�� step �� actionIndex
            StepID nextStepID;
            branch->getNextStepID(nextStepID);
            user.curStepID = nextStepID;
            user.actIndex = -1;
            break;
        }
        // �û������룬�ҵ�ǰΪ default ��֧
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