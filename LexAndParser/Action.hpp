#pragma once
#include "ParseTree.h"

/// @brief 动作的抽象类
class Action
{
public:
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

    /// @brief 纯虚函数，由子类实现
    /// @return 当前动作类型
    virtual ActionType getCurType() const = 0;

protected:
    ActionType curType = Action::ActionType::UnKnown;
};

/// @brief 动作：输出
class Out : public Action
{
private:
    /// @brief Out expression
    ///       [ expression -> string | var | expression + expression ]
    Expression outInfo;
public:
    Out(Expression outInfo_)
    {
        curType = ActionType::Out;
        this->outInfo = outInfo_;
    }
    ActionType getCurType() const { return curType; }
    void getOutInfo(Expression& result)
    {
        result = outInfo;
    }
};

/// @brief 动作：等待用户答复
class Listen : public Action
{
private:
    /// @brief Listen listenTime(number) 单位为 s
    int listenTime = 0;
public:
    Listen(int listenTime_)
    {
        curType = ActionType::Listen;
        this->listenTime = listenTime_;
    }
    ActionType getCurType() const { return curType; }
    int getListenTime() { return listenTime; };
};

/// @brief 动作：分支跳转
class Branch : public Action
{
private:
    /// @brief Branch keyWord(string) nextStep
    KeyWord keyWord;
    /// @brief Branch keyWord(string) nextStep
    StepID nextStep;
public:
    Branch(KeyWord keyWord_, StepID nextStep_)
    {
        curType = ActionType::Branch;
        this->keyWord = keyWord_;
        this->nextStep = nextStep_;
    }
    ActionType getCurType() const { return curType; }
    void getKeyWord(KeyWord& result) { result = keyWord; }
    void getNextStepID(StepID& result) { result = nextStep; }
};

/// @brief 动作：判断用户沉默
class Silence : public Action
{
private:
    /// @brief Silence nextStepID
    StepID nextStepID;
public:
    Silence(StepID next) : nextStepID(next)
    {
        curType = Action::ActionType::Silence;
    }
    ActionType getCurType() const { return curType; }
    void getNextStepID(StepID& result) { result = nextStepID; }
};

/// @brief 动作：默认分支
class Default : public Action
{
private:
    /// @brief Default nextStepID
    StepID nextStepID;
public:
    Default(StepID next) : nextStepID(next)
    {
        curType = Action::ActionType::Default;
    }
    ActionType getCurType() const { return curType; }
    void getNextStepID(StepID& result) { result = nextStepID; }
};

/// @brief 动作：退出
class Exit : public Action
{
public:
    Exit() { curType = ActionType::Exit; }
    ActionType getCurType() const { return curType; }
};
