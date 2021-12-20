#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Interpreter.h"


using UserID = std::string;
using UserTable = std::unordered_map<UserID, std::unique_ptr<User>>;
using UserPair = std::pair<UserID, std::unique_ptr<User>>;

class ScriptServer
{
public:
    /// @brief 创建新用户结果状态
    enum class NEW_USER_STA
    {
        WrongIDFormat,
        AlreadyExists,
        Succeed
    };

    /// @brief 删除用户结果状态
    enum class DEL_USER_STA
    {
        NoSuchUser,
        Succeed
    };

    /// @brief 解释器需要外部执行的动作状态
    enum class INTERPRET_STA
    {
        Do,
        Listen,
        Exit,
        Out,
    };

    /// @brief 用于测试的输入文件
    std::ifstream inFile;

    /// @brief 用于测试的输出文件
    std::string logName;

    ScriptServer(const char* scriptName_);

    ~ScriptServer();

    /// @brief 初始化脚本服务器
    bool init();

    /// @brief 根据用户 id 创建新用户
    ScriptServer::NEW_USER_STA createUser(UserID ID_);

    /// @brief 根据用户 id 删除用户
    ScriptServer::DEL_USER_STA deleteUser(UserID& ID_);

    /// @brief 自然语言处理模块接口
    void msgToUserInputKey(UserID& ID_, std::string msg);

    /// @brief 用户 ID_ 执行脚本
    ScriptServer::INTERPRET_STA srvInterpret(UserID& ID_);

    /// @brief 获取要发送给用户的信息
    void getOutputMsg(UserID& ID_, std::string& outputMsg);

    /// @brief 设置需要外部执行的动作已完成
    void setFinished(UserID& ID_);
private:

    /// @brief 输入输出封装
    DBG dbg;

    /// @brief 脚本路径
    const char* scriptPath;

    /// @brief 封装脚本解释器
    Interpreter interpreter;

    /// @brief 所有用户
    UserTable users;

    /// @brief 判断用户 ID 是否有效
    bool isValidFormat(UserID& id);

    /// @brief 错误处理模块
    void error(ScriptServer::NEW_USER_STA err);
};
