#include "Parser.h"
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

const int LINE_BUFFER_SIZE = 300;
const std::string BLANKS("\f\v\r\t\n ");
const char ANNOTATION_SYMBOL = '#';

/// @brief 删除字符串 str 首尾的空白字符（\f \v \r \t \n）
///        删除行内的注释（以 # 开头到末尾的所有内容）
/// @param str 
void Parser::trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(BLANKS));  // 删除行首空白字符
    str.erase(str.find_last_not_of(BLANKS) + 1);  // 删除行尾空白字符
    str.erase(str.find_last_not_of(ANNOTATION_SYMBOL) + 1); //删除注释，包括单行注释和行内注释
}


/// @brief 对脚本文件进行语法分析，负责打开、关闭文件，将文件分行
void Parser::parseFile() {
    std::ifstream inFile;
    inFile.open(scriptName);
    char lineBuf[LINE_BUFFER_SIZE];
    inFile.getline(lineBuf, LINE_BUFFER_SIZE);
    Line line = lineBuf;
    trim(line); // 删除行首和行尾的空白字符，删除行内的注释
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

//todo  可以使用设计模式进行改进

/// @brief 根据每行的第一个 token 确定下一步的动作
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
