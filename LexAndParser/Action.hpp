#pragma once
#include "ParseTree.h"

/// @brief �����ĳ�����
class Action
{
protected:
    /// @brief ���ж���������
    enum class ActionType
    {
        Out,
        Listen,
        Branch,
        Silence,
        Default,
        Exit,
        UnKnown
    };
    ActionType curType;
    virtual ActionType getCurType() const = 0;
};

/// @brief ���������
class Out : public Action {
private:
    Expression outInfo;
public:
    Out(Expression outInfo_) {
        curType = ActionType::Out;
        this->outInfo = outInfo_;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief �������ȴ��û���
class Listen : public Action {
private:
    int listenTime = 0;
public:
    Listen(int listenTime_) {
        curType = ActionType::Listen;
        this->listenTime = listenTime_;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief �������жϷ�֧�Ĺؼ���
class Branch : public Action {
private:
    KeyWord keyWord;
    StepID nextStep;
public:
    Branch(KeyWord keyWord_, StepID nextStep_) {
        curType = ActionType::Branch;
        this->keyWord = keyWord_;
        this->nextStep = nextStep_;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief �������ж��û��Ƿ��лظ�
class Silence : public Action {
private:
    StepID nextStepID;
public:
    Silence(StepID next) : nextStepID(next) {
        curType = Action::ActionType::Silence;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief ������Ĭ�Ϸ�֧
class Default : public Action {
private:
    StepID nextStepID;
public:
    Default(StepID next) : nextStepID(next) {
        curType = Action::ActionType::Default;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief �������˳�
class Exit : public Action {
public:
    Exit() { curType = ActionType::Exit; }
    ActionType getCurType() const { return curType; }
};
