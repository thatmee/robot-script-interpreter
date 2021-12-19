#include "DBG.h"
void DBG::out(std::string msg)
{
    std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_)
{
    if (dbgLevel_ == dbgLevel)
        std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::ofstream& outFile)
{
    if (dbgLevel_ == dbgLevel && outFile.is_open())
        outFile << msg;
    else if (dbgLevel_ == this->dbgLevel)
        std::cout << msg;
}
void DBG::out(std::string msg, std::ofstream& outFile)
{
    if (outFile.is_open())
        outFile << msg;
    else
        std::cout << msg;
}

void DBG::getline(std::string& resultMsg)
{
    std::getline(std::cin, resultMsg);
}

void DBG::getline(std::ifstream& inFile, int size, std::string& resultMsg)
{
    if (inFile.is_open())
    {
        char* buffer = new char[size];
        inFile.getline(buffer, size, '\n');
        resultMsg = std::string(buffer);
    }
    else
        std::getline(std::cin, resultMsg);
}

void DBG::setDbgLevel(DBG::DBG_LEVELS dbgLevel_)
{
    this->dbgLevel = dbgLevel_;
}

void DBG::setOutPipe(DBG::PIPES outPipe_)
{
    this->outPipe = outPipe_;
}