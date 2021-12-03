#include "Parser.h"
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

const int LINE_BUFFER_SIZE = 300;
const std::string BLANKS("\f\v\r\t\n ");
const char ANNOTATION_SYMBOL = '#';

/// @brief ɾ���ַ��� str ��β�Ŀհ��ַ���\f \v \r \t \n��
///        ɾ�����ڵ�ע�ͣ��� # ��ͷ��ĩβ���������ݣ�
/// @param str 
void Parser::trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(BLANKS));  // ɾ�����׿հ��ַ�
    str.erase(str.find_last_not_of(BLANKS) + 1);  // ɾ����β�հ��ַ�
    str.erase(str.find_last_not_of(ANNOTATION_SYMBOL) + 1); //ɾ��ע�ͣ���������ע�ͺ�����ע��
}


/// @brief �Խű��ļ������﷨����������򿪡��ر��ļ������ļ�����
void Parser::parseFile() {
    std::ifstream inFile;
    inFile.open(scriptName);
    char lineBuf[LINE_BUFFER_SIZE];
    inFile.getline(lineBuf, LINE_BUFFER_SIZE);
    Line line = lineBuf;
    trim(line); // ɾ�����׺���β�Ŀհ��ַ���ɾ�����ڵ�ע��
    if (line.size() > 0)
        parseLine(line);
    inFile.close();
}

/// @brief 
/// @param line 
void Parser::parseLine(Line line) {
    boost::split(tokenStream, line, boost::is_any_of(BLANKS), boost::token_compress_on);
    procTokens();
}

//todo  ����ʹ�����ģʽ���иĽ�

/// @brief ����ÿ�еĵ�һ�� token ȷ����һ���Ķ���
void Parser::procTokens() {
    Token lineKey = tokenStream.at(0);
    if (lineKey == "Step")
        procStep();
    else if (lineKey == "Out")
        procOut();
    else if (lineKey == "Listen")
        procListen();
    else if (lineKey == "Branch")
        procBranch();
    else if (lineKey == "Silence")
        procSilence();
    else if (lineKey == "Default")
        procDefault();
    else if (lineKey == "Exit")
        procExit();
    else error(ERR_STATE::UnknownToken);
}

void Parser::procStep() {

}
