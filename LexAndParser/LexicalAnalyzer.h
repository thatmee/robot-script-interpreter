#pragma once
#include <string>

/// @brief 词法分析器类
class LexAna
{
private:
    /// @brief 词法分析器的所有状态
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
    };

    /// @brief 要检查的词语
    std::string word;

    /// @brief 当前指向的字符
    char ch;

    /// @brief 向前指针
    int index;

    /// @brief 移动向前指针指向下一个字符，更新 ch 的值
    void forwardPointer();

    /// @brief 判断当前 ch 是不是字母（a-z, A-Z）
    bool isLetter();

    /// @brief 判断当前 ch 是不是数字 (0-9)
    bool isDigit();

    /// @brief 错误处理函数
    void error();

public:
    /// @brief 本词法分析器能够识别的类型
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

    /// @brief 对当前词语进行分析，一个词法分析器类对应一个词语，但是可以通过 setWord 设置新的词语
    /// @return 返回分析出的词语类型
    TYPE checkLexical();
};
