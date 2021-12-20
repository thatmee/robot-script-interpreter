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
    /// @brief �û�Ψһ��ʶ������Ϊ 3 ��λ��Ϊ 0 ������
    UserID ID = "";

    /// @brief �û���������ȱʡ
    UserName name = "";

    /// @brief �û���ǰִ�е��ű��Ĳ���
    StepID curStepID = "";

    /// @brief �û���ǰִ�е���ĳһ����Ķ������
    int actIndex = -1;

    /// @brief �û�������
    VarTable vars;

    /// @brief �û�����Ĺؼ���
    std::string inputKey = "";

    /// @brief ��Ҫ�ⲿִ�еĶ����Ƿ������
    bool finished = true;

    /// @brief Ҫ������û�����Ϣ
    std::string outputMsg = "";

    /// @brief ����ʱ������
    int listenTime = -1;

    User(UserID ID_, UserName name_ = "") : ID(ID_), name(name_) {}

    ~User();
};
