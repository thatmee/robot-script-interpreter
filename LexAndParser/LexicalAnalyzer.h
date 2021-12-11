#pragma once
#include <string>

class LexAna
{
private:
    enum class LEXI_STATE
    {
        Entry,
        Identifier,
        Zero,
        Number,
        Add,
        ConstStrBegin,
        ConstStrEnd,
        Variable,
        Colon,
        Comma,
        ESC
        //Err,
        //Exit
    };
    std::string word;
    char ch;
    int index;
    void forwardPointer();
    bool isLetter();
    bool isDigit();
    void error();

public:
    enum class TYPE
    {
        Identifier,
        Zero,
        Number,
        ConstStr,
        Punc,
        Err,
        Empty
    };
    LexAna(std::string word) : word(word), index(0), ch('\0') {}
    void setWord(std::string word) {
        this->word = word;
        index = 0;
        ch = '\0';
    }
    TYPE checkLexical();
};
