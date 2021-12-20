#pragma once
#include <string>
#include <unordered_map>
#include "LexAndParser/ParseTree.h"

using UserID = std::string;
using UserName = std::string;
using VarTable = std::unordered_map<VarName, std::string>;
using VarTablePair = std::pair<VarName, std::string>;

class User
{
public:
    /// @brief 用户唯一标识，长度为 3 首位不为 0 的数字
    UserID ID = "";

    /// @brief 用户名，可以缺省
    UserName name = "";

    /// @brief 用户当前执行到脚本的步骤
    StepID curStepID = "";

    /// @brief 用户当前执行到的某一步骤的动作序号
    int actIndex = -1;

    /// @brief 用户变量表
    VarTable vars;

    /// @brief 用户输入的关键词
    std::string inputKey = "";

    /// @brief 需要外部执行的动作是否已完成
    bool finished = true;

    /// @brief 要输出给用户的信息
    std::string outputMsg = "";

    /// @brief 监听时间上限
    int listenTime = -1;

    User(UserID ID_, UserName name_ = "") : ID(ID_), name(name_) {}

    ~User();
};
