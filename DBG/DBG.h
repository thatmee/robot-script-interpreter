#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "test.h"

class DBG
{
public:
    /// @brief ������Ϣ����ȼ�
    enum class DBG_LEVELS
    {
        Simple,
        Detail,
        Test
    };

    /// @brief ������Ϣ����ܵ�
    enum class PIPES
    {
        Standard,
        FileIO,
    };

    DBG() {}

    DBG(DBG::DBG_LEVELS classDbgLevel) : dbgLevel(classDbgLevel) {}

    ~DBG() {}

    /// @brief ���
    void out(std::string msg);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::string outFileName);
    void out(std::string msg, std::string outFileName);

    /// @brief ����
    void getline(std::string& resultMsg);
    void getline(std::ifstream& inFile, int size, std::string& resultMsg);

    /// @brief ���õ��Լ���
    void setDbgLevel(DBG::DBG_LEVELS dbgLevel_);

    /// @brief ��������ܵ�
    void setOutPipe(DBG::PIPES outPipe_);

    /// @brief ���� fullPath �õ� log �ļ���
    void anaLogName(const char* fullPath, std::string& fileName);

    /// @brief ���� fullPath �õ� input �ļ���
    void anaInfileName(const char* fullPath, std::string& fileName);

private:
    DBG::DBG_LEVELS dbgLevel = DBG::DBG_LEVELS::Simple;
    DBG::PIPES outPipe = DBG::PIPES::Standard;
};

