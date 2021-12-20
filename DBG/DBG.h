#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "test.h"

class DBG
{
public:
    /// @brief 调试信息输出等级
    enum class DBG_LEVELS
    {
        Simple,
        Detail,
        Test
    };

    /// @brief 调试信息输出管道
    enum class PIPES
    {
        Standard,
        FileIO,
    };

    DBG() {}

    DBG(DBG::DBG_LEVELS classDbgLevel) : dbgLevel(classDbgLevel) {}

    ~DBG() {}

    /// @brief 输出
    void out(std::string msg);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::string outFileName);
    void out(std::string msg, std::string outFileName);

    /// @brief 输入
    void getline(std::string& resultMsg);
    void getline(std::ifstream& inFile, int size, std::string& resultMsg);

    /// @brief 设置调试级别
    void setDbgLevel(DBG::DBG_LEVELS dbgLevel_);

    /// @brief 设置输出管道
    void setOutPipe(DBG::PIPES outPipe_);

    /// @brief 根据 fullPath 得到 log 文件名
    void anaLogName(const char* fullPath, std::string& fileName);

    /// @brief 根据 fullPath 得到 input 文件名
    void anaInfileName(const char* fullPath, std::string& fileName);

private:
    DBG::DBG_LEVELS dbgLevel = DBG::DBG_LEVELS::Simple;
    DBG::PIPES outPipe = DBG::PIPES::Standard;
};

