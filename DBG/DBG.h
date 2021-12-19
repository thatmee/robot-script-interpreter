#pragma once
#include <string>
#include <iostream>
#include <fstream>

class DBG
{
public:
    enum class DBG_LEVELS
    {
        Simple,
        Detail
    };
    enum class PIPES
    {
        Standard,
        FileIO,
    };
    DBG() {}
    DBG(DBG::DBG_LEVELS classDbgLevel) : dbgLevel(classDbgLevel) {}
    ~DBG() {}
    void out(std::string msg);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_);
    void out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::ofstream& outFile);
    void out(std::string msg, std::ofstream& outFile);
    void getline(std::string& resultMsg);
    void getline(std::ifstream& inFile, int size, std::string& resultMsg);
    void setDbgLevel(DBG::DBG_LEVELS dbgLevel_);
    void setOutPipe(DBG::PIPES outPipe_);

private:
    DBG::DBG_LEVELS dbgLevel = DBG::DBG_LEVELS::Simple;
    DBG::PIPES outPipe = DBG::PIPES::Standard;
};

