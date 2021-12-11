#pragma once
#include "ParseTree.h"

/// @brief 动作的抽象类
class Action
{
protected:
    /// @brief 所有动作的类型
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

/// @brief 动作：输出
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

/// @brief 动作：等待用户答复
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

/// @brief 动作：判断分支的关键词
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

/// @brief 动作：判断用户是否有回复
class Silence : public Action {
private:
    StepID nextStepID;
public:
    Silence(StepID next) : nextStepID(next) {
        curType = Action::ActionType::Silence;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief 动作：默认分支
class Default : public Action {
private:
    StepID nextStepID;
public:
    Default(StepID next) : nextStepID(next) {
        curType = Action::ActionType::Default;
    }
    ActionType getCurType() const { return curType; }
};

/// @brief 动作：退出
class Exit : public Action {
public:
    Exit() { curType = ActionType::Exit; }
    ActionType getCurType() const { return curType; }
};
