#include "DBG.h"
void DBG::out(std::string msg)
{
    // 忽略调试级别，永远标准输出
    std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_)
{
    // 调试级别符合，永远标准输出
    if (dbgLevel_ == dbgLevel)
        std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::string outFileName)
{
    // 调试级别符合，不使用文件输出
    if (dbgLevel_ == this->dbgLevel && outPipe != DBG::PIPES::FileIO)
    {
        std::cout << msg;
        return;
    }

    std::ofstream outFile;
    outFile.open(outFileName, std::ios::app);
    // 调试级别符合，使用文件输出
    if (dbgLevel_ == dbgLevel && outFile.is_open() && outPipe == DBG::PIPES::FileIO)
    {
        outFile << msg;
        outFile.close();
    }
}
void DBG::out(std::string msg, std::string outFileName)
{
    // 忽略调试级别，不使用文件输出
    if (outPipe != DBG::PIPES::FileIO)
    {
        std::cout << msg;
        return;
    }
    std::ofstream outFile;
    outFile.open(outFileName, std::ios::app);
    // 忽略调试级别，使用文件输出
    if (outFile.is_open())
    {
        outFile << msg;
        outFile.close();
    }
}

void DBG::getline(std::string& resultMsg)
{
    // 标准 getline
    std::getline(std::cin, resultMsg);
}

void DBG::getline(std::ifstream& inFile, int size, std::string& resultMsg)
{
    // 从文件中 getline
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

void DBG::anaLogName(const char* fullPath, std::string& fileName)
{
    // 从脚本文件路径获取当前脚本文件的名称
    std::string pathStr(fullPath);
    std::string scriptName;
    std::string::size_type sep = pathStr.find_last_of("/");
    std::string::size_type suf = pathStr.find_last_of(".");
    if (sep == std::string::npos)
        sep = 0;
    if (suf == std::string::npos)
        scriptName = pathStr.substr(sep);
    else if (sep < suf)
        scriptName = pathStr.substr(sep + 1, suf - sep - 1);
    else
        scriptName = "error";

    // 测试日志文件名称为：脚本文件名称.log
    fileName = "./log/" + scriptName + ".log";
}

void DBG::anaInfileName(const char* fullPath, std::string& fileName)
{
    // 从脚本文件路径获取当前脚本文件的名称
    std::string pathStr(fullPath);
    std::string scriptName;
    std::string::size_type sep = pathStr.find_last_of("/");
    std::string::size_type suf = pathStr.find_last_of(".");
    if (sep == std::string::npos)
        sep = 0;
    if (suf == std::string::npos)
        scriptName = pathStr.substr(sep);
    else if (sep < suf)
        scriptName = pathStr.substr(sep + 1, suf - sep - 1);
    else
        scriptName = "input";

    fileName = "./input/" + scriptName + ".in";
}