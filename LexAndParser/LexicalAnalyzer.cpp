#include "LexicalAnalyzer.h"


void LexAna::forwardPointer()
{
    if (index < word.size())
    {
        ch = word.at(index);
        index++;
    }
    else
        ch = '\0';
}


bool LexAna::isLetter()
{
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        return true;
    else
        return false;
}


bool LexAna::isDigit()
{
    if (ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}


void LexAna::error()
{

}


LexAna::TYPE LexAna::checkLexical()
{
    // 实现逻辑为词法的状态转换图，详见开发文档

    LEXI_STATE state = LexAna::LEXI_STATE::Entry;
    while (1)
    {
        switch (state)
        {
        case LexAna::LEXI_STATE::Entry:
            forwardPointer();
            switch (ch)
            {
            case '0':
                state = LexAna::LEXI_STATE::Zero;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                state = LexAna::LEXI_STATE::Number;
                break;
            case '+':
                state = LexAna::LEXI_STATE::Add;
                break;
            case '"':
                state = LexAna::LEXI_STATE::ConstStrBegin;
                break;
            case '$':
                state = LexAna::LEXI_STATE::Variable;
                break;
            case ':':
                state = LexAna::LEXI_STATE::Colon;
                break;
            case ',':
                state = LexAna::LEXI_STATE::Comma;
                break;
            case '\\':
                state = LexAna::LEXI_STATE::ESC;
                break;
            case '\0':
                return LexAna::TYPE::Empty;
            default:
                if (isLetter())
                    state = LexAna::LEXI_STATE::Identifier;
                else
                {
                    error();
                    return LexAna::TYPE::Err;
                }
                break;
            }
            break;
        case LexAna::LEXI_STATE::Identifier:
            forwardPointer();
            if (isLetter() || isDigit() || ch == '_')
                state = LexAna::LEXI_STATE::Identifier;
            else if (ch == '\0')
                return LexAna::TYPE::Identifier;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::Zero:
            forwardPointer();
            if (ch == '\0')
                return LexAna::TYPE::Zero;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::Number:
            forwardPointer();
            if (isDigit())
                state = state;
            else if (ch == '\0')
                return LexAna::TYPE::Number;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::ConstStrBegin:
            forwardPointer();
            if (ch == '"')
                state = LexAna::LEXI_STATE::ConstStrEnd;
            else if (ch == '\\')
                state = LexAna::LEXI_STATE::ESC;
            else if (ch == '\0')
            {
                error();
                return LexAna::TYPE::Err;
            }
            else
                state = state;
            break;
        case LexAna::LEXI_STATE::ConstStrEnd:
            forwardPointer();
            if (ch == '\0')
                return LexAna::TYPE::ConstStr;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::Variable:
            forwardPointer();
            if (isLetter() || isDigit() || ch == '_')
                state = LexAna::LEXI_STATE::Identifier;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::Add:
        case LexAna::LEXI_STATE::Colon:
        case LexAna::LEXI_STATE::Comma:
            forwardPointer();
            if (ch == '\0')
                return LexAna::TYPE::Punc;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        case LexAna::LEXI_STATE::ESC:
            forwardPointer();
            if (ch == '"' || ch == '\\')
                state = LexAna::LEXI_STATE::ConstStrBegin;
            else
            {
                error();
                return LexAna::TYPE::Err;
            }
            break;
        }
    }
}
