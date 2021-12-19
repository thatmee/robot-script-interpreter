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

/// @brief �﷨������
class Parser
{
private:
    /// @brief ���д���״̬
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

    /// @brief �����жϱ��ʽ�﷨��ȷ��
    enum class EXPR_STATE
    {
        WaitAdd,
        WaitToken
    };


    /// @brief ��ǰɨ��Ľű��к�
    int lineCnt;

    /// @brief ����һ�нű��� token ����
    TokenStream tokenStream;
    StepID curStepID = "";

    /// @brief Token ����ͷ������һ��Ԫ��
    /// @param stream Ҫ���ӵ� Token ����
    /// @return ���ӵ�Ԫ��
    Token popFront(TokenStream& stream);


    /// @brief ɾ���ַ�����β�Ŀհ��ַ���
    ///        ɾ��ע�ͣ���������ע�ͺ�����ע��
    /// @param str Ҫ�޼����ַ���
    void trim(std::string& str);

    /// @brief �ָ��ַ�������������� vector ��
    /// @param s Ҫ�ָ���ַ���
    /// @param result �ָ����� vector
    /// @param delim �ָ��������԰�������ַ�
    void splitString(const std::string& s, std::vector<std::string>& result, const std::string& delim);

    /// @brief �Խű��ļ������﷨����������򿪡��ر��ļ������ļ����к�����һ����������
    void parseFile();

    /// @brief ��һ�нű��ָ�� Token ���У������� tokenStream ��
    /// @param line Ҫ�ָ�Ľű�
    void parseLine(Line line);

    /// @brief ���� tokenStream �ĵ�һ�� token ȷ����һ���Ķ���
    void procTokens();

    /// @brief ���� Step ���
    void procStep();

    /// @brief ���� Out ���
    void procOut();

    /// @brief ���� Listen ���
    void procListen();

    /// @brief ���� Branch ���
    void procBranch();

    /// @brief ���� Silence ���
    void procSilence();

    /// @brief ���� Default ���
    void procDefault();

    /// @brief ���� Exit ���
    void procExit();

    /// @brief ������ģ��
    /// @param err �����Ĵ�������
    void error(ERR_STATE err);


protected:
    /// @brief Ҫ�����Ľű��ļ�·����һ�� Parser ���Ӧһ���ű�
    const char* scriptPath;

#ifdef PARSE_GTEST
    std::ofstream logFile;
#endif // PARSE_GTEST


public:
    /// @brief �﷨���������������﷨��
    ParseTree parseTree;

    Parser() : scriptPath(nullptr), lineCnt(1) {}
    Parser(const char* scriptPath_);
    ~Parser();

    /// @brief ���ݽű��ļ������﷨��
    void generateParseTree();
};
