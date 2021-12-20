#pragma once
#include "ParseTree.h"

/// @brief �����ĳ�����
class Action
{
public:
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

    /// @brief ���麯����������ʵ��
    /// @return ��ǰ��������
    virtual ActionType getCurType() const = 0;

protected:
    ActionType curType = Action::ActionType::UnKnown;
};

/// @brief ���������
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

/// @brief �������ȴ��û���
class Listen : public Action
{
private:
    /// @brief Listen listenTime(number) ��λΪ s
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

/// @brief ��������֧��ת
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

/// @brief �������ж��û���Ĭ
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

/// @brief ������Ĭ�Ϸ�֧
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

/// @brief �������˳�
class Exit : public Action
{
public:
    Exit() { curType = ActionType::Exit; }
    ActionType getCurType() const { return curType; }
};
