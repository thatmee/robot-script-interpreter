#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <lib\magic_enum.hpp>
#include <codecvt>
#include <regex>
#include "ParseTree.h"
#include "LexicalAnalyzer.h"
#include "Action.hpp"
#define PARSE_GTEST

using Line = std::string;
using Token = std::string;
using TokenStream = std::vector<Token>;

const int LINE_BUFFER_SIZE = 300;
const std::string BLANKS = "\f\v\r\t ";
const char ANNOTATION_SYMBOL = '#';
const Token STREAM_EMPTY = "00";

/// @brief 语法分析类
class Parser
{
private:
    /// @brief 所有错误状态
    enum class ERR_STATE
    {
        UnknownToken,
        DuplicatedStep,
        TooManyTokens,
        TooFewTokens,
        LexicalError,
        WrongExprssion,
        NoCorrespondingStep,
        WrongType
    };

    /// @brief 用于判断表达式语法正确性
    enum class EXPR_STATE
    {
        WaitAdd,
        WaitToken
    };


    /// @brief 当前扫描的脚本行号
    int lineCnt;

    /// @brief 保存一行脚本的 token 序列
    TokenStream tokenStream;
    StepID curStepID = "";

    /// @brief Token 队列头部出队一个元素
    /// @param stream 要出队的 Token 队列
    /// @return 出队的元素
    Token popFront(TokenStream& stream);


    /// @brief 删除字符串首尾的空白字符；
    ///        删除注释，包括单行注释和行内注释
    /// @param str 要修剪的字符串
    void trim(std::string& str);

    /// @brief 分割字符串，结果保存在 vector 中
    /// @param s 要分割的字符串
    /// @param result 分割结果的 vector
    /// @param delim 分隔符，可以包含多个字符
    void splitString(const std::string& s, std::vector<std::string>& result, const std::string& delim);

    /// @brief 对脚本文件进行语法分析，负责打开、关闭文件，对文件分行后交由下一个函数处理
    void parseFile();

    /// @brief 将一行脚本分割成 Token 序列，保存在 tokenStream 中
    /// @param line 要分割的脚本
    void parseLine(Line line);

    /// @brief 根据 tokenStream 的第一个 token 确定下一步的动作
    void procTokens();

    /// @brief 处理 Step 语句
    void procStep();

    /// @brief 处理 Out 语句
    void procOut();

    /// @brief 处理 Listen 语句
    void procListen();

    /// @brief 处理 Branch 语句
    void procBranch();

    /// @brief 处理 Silence 语句
    void procSilence();

    /// @brief 处理 Default 语句
    void procDefault();

    /// @brief 处理 Exit 语句
    void procExit();

    /// @brief 错误处理模块
    /// @param err 发生的错误类型
    void error(ERR_STATE err);


protected:
    /// @brief 要分析的脚本文件路径，一个 Parser 类对应一个脚本
    const char* scriptPath;

#ifdef PARSE_GTEST
    std::ofstream logFile;
#endif // PARSE_GTEST


public:
    /// @brief 语法分析结果所保存的语法树
    ParseTree parseTree;

    Parser() : scriptPath(nullptr), lineCnt(1) {}
    Parser(const char* scriptPath_);
    ~Parser();

    /// @brief 根据脚本文件生成语法树
    void generateParseTree();
};
