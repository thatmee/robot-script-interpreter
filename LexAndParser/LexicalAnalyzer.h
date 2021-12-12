#pragma once
#include <string>

/// @brief �ʷ���������
class LexAna
{
private:
    /// @brief �ʷ�������������״̬
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

    /// @brief Ҫ���Ĵ���
    std::string word;

    /// @brief ��ǰָ����ַ�
    char ch;

    /// @brief ��ǰָ��
    int index;

    /// @brief �ƶ���ǰָ��ָ����һ���ַ������� ch ��ֵ
    void forwardPointer();

    /// @brief �жϵ�ǰ ch �ǲ�����ĸ��a-z, A-Z��
    bool isLetter();

    /// @brief �жϵ�ǰ ch �ǲ������� (0-9)
    bool isDigit();

    /// @brief ��������
    void error();

public:
    /// @brief ���ʷ��������ܹ�ʶ�������
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

    /// @brief �Ե�ǰ������з�����һ���ʷ����������Ӧһ��������ǿ���ͨ�� setWord �����µĴ���
    /// @return ���ط������Ĵ�������
    TYPE checkLexical();
};
