#include "pch.h"
#include "LexicalAnalyzer.h"
#include "Parser.h"

TEST(TestCaseName, TestName) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

TEST(checkLexical, Identifier) {
    std::string iden = "abc123_";
    std::string wrongIden = "_abc";

    EXPECT_TRUE(LexAna::TYPE::Identifier == LexAna(iden).checkLexical());
    EXPECT_TRUE(LexAna::TYPE::Err == LexAna(wrongIden).checkLexical());

}