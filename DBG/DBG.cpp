#include "DBG.h"
void DBG::out(std::string msg)
{
    // ���Ե��Լ�����Զ��׼���
    std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_)
{
    // ���Լ�����ϣ���Զ��׼���
    if (dbgLevel_ == dbgLevel)
        std::cout << msg;
}
void DBG::out(std::string msg, DBG::DBG_LEVELS dbgLevel_, std::string outFileName)
{
    // ���Լ�����ϣ���ʹ���ļ����
    if (dbgLevel_ == this->dbgLevel && outPipe != DBG::PIPES::FileIO)
    {
        std::cout << msg;
        return;
    }

    std::ofstream outFile;
    outFile.open(outFileName, std::ios::app);
    // ���Լ�����ϣ�ʹ���ļ����
    if (dbgLevel_ == dbgLevel && outFile.is_open() && outPipe == DBG::PIPES::FileIO)
    {
        outFile << msg;
        outFile.close();
    }
}
void DBG::out(std::string msg, std::string outFileName)
{
    // ���Ե��Լ��𣬲�ʹ���ļ����
    if (outPipe != DBG::PIPES::FileIO)
    {
        std::cout << msg;
        return;
    }
    std::ofstream outFile;
    outFile.open(outFileName, std::ios::app);
    // ���Ե��Լ���ʹ���ļ����
    if (outFile.is_open())
    {
        outFile << msg;
        outFile.close();
    }
}

void DBG::getline(std::string& resultMsg)
{
    // ��׼ getline
    std::getline(std::cin, resultMsg);
}

void DBG::getline(std::ifstream& inFile, int size, std::string& resultMsg)
{
    // ���ļ��� getline
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
    // �ӽű��ļ�·����ȡ��ǰ�ű��ļ�������
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

    // ������־�ļ�����Ϊ���ű��ļ�����.log
    fileName = "./log/" + scriptName + ".log";
}

void DBG::anaInfileName(const char* fullPath, std::string& fileName)
{
    // �ӽű��ļ�·����ȡ��ǰ�ű��ļ�������
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