#include <iostream>
#include <fstream>
#include <string>
#include "pch.h"
#include "LexAndParser/LexicalAnalyzer.h"
#include "LexAndParser/Parser.h"

TEST(checkLexical, Identifier) {
    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna("abc123_").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna("Abjh3901__3jsienBESIds").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna("$0abc").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna("$abc_1").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna("$_name").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("_abc").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("0abc_").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("name-1").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("#name1").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("name#1").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("$").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("$*abc-2").checkLexical());
}

TEST(checkLexical, Zero) {
    EXPECT_TRUE(LexAna::TYPE::Zero == LexAna("0").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("00").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("012").checkLexical());
}

TEST(checkLexical, Number) {
    EXPECT_TRUE(LexAna::TYPE::Number == LexAna("123").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("1234569c23").checkLexical());
}

TEST(checkLexical, ConstStr) {
    EXPECT_TRUE(LexAna::TYPE::ConstStr == LexAna("\"hello, welcome.\"").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::ConstStr == LexAna("\"call: \\\"88888\\\".\"").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::ConstStr == LexAna("\"test \\\\.\"").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("\"hello, welcome.").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("\"hello\\, welcome.\"").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("\"hello, welcome.\"out words").checkLexical());
}

TEST(checkLexical, Punc) {
    EXPECT_TRUE(LexAna::TYPE::Punc == LexAna(",").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Punc == LexAna("+").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Punc == LexAna(":").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna(",,").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("++").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("-").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna(";").checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna("*").checkLexical());
}

TEST(checkLexical, Empty) {
    EXPECT_TRUE(LexAna::TYPE::Empty == LexAna("").checkLexical());
}

TEST(parser, correctTest) {
    Parser p("./test_scripts/unit-test-correct.rsl");
    p.generateParseTree();
    std::ifstream sampleLog, realLog;
    sampleLog.open("./test_scripts/sample_log/unit-test-correct.rsl.log");
    realLog.open("./log/unit-test-correct.rsl.log");
    std::string sampleStr, realStr;
    while (!sampleLog.eof())
        sampleStr += sampleLog.get();
    while (!realLog.eof())
        realStr += realLog.get();
    sampleLog.close();
    realLog.close();
    EXPECT_TRUE(sampleStr == realStr);
}

TEST(parser, wrongTest) {
    Parser p("./test_scripts/unit-test-wrong.rsl");
    p.generateParseTree();
    std::ifstream sampleLog, realLog;
    sampleLog.open("./test_scripts/sample_log/unit-test-wrong.rsl.log");
    realLog.open("./log/unit-test-wrong.rsl.log");
    std::string sampleStr, realStr;
    while (!sampleLog.eof())
        sampleStr += sampleLog.get();
    while (!realLog.eof())
        realStr += realLog.get();
    sampleLog.close();
    realLog.close();
    EXPECT_TRUE(sampleStr == realStr);
}