#include "Parser.h"

const int LINE_BUFFER_SIZE = 300;
const std::string BLANKS("\f\v\r\t ");
const char ANNOTATION_SYMBOL = '#';
const Token STREAM_EMPTY = "00";

Token Parser::popFront(TokenStream& stream) {
    if (stream.empty())
        return STREAM_EMPTY;
    Token front = stream.at(0);
    stream.erase(stream.begin());
    return front;
}

/// @brief 删除字符串 str 首尾的空白字符（\f \v \r \t ）
///        删除注释，包括单行注释和行内注释
/// @param str 
void Parser::trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(BLANKS));  // 删除行首空白字符
    str.erase(str.find_last_not_of(BLANKS) + 1);  // 删除行尾空白字符
    str.erase(str.find_last_not_of(ANNOTATION_SYMBOL) + 1); //删除注释，包括单行注释和行内注释
}

/// @brief 
/// @param s 
/// @param result 
/// @param delim 
void Parser::splitString(const std::string& s, std::vector<std::string>& result, const std::string& delim)
{
    std::string::size_type pos1, pos2;
    std::string constStr = "";
    pos2 = s.find_first_of(delim);
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        std::string tmp = s.substr(pos1, pos2 - pos1);
        if (!tmp.empty()) {
            result.push_back(tmp);
        }
        pos1 = s.find_first_not_of(delim, pos2 + 1);
        pos2 = s.find_first_of(delim, pos1);
    }
    if (pos1 != s.length())
        result.push_back(s.substr(pos1));
}

/// @brief 对脚本文件进行语法分析，负责打开、关闭文件，将文件分行
void Parser::parseFile() {
    std::ifstream inFile;
    inFile.open(scriptName);
    // 逐行读取脚本文件
    while (!inFile.eof()) {
        char lineBuf[LINE_BUFFER_SIZE];
        inFile.getline(lineBuf, LINE_BUFFER_SIZE);
        Line line = lineBuf;
        // 删除行首和行尾的空白字符，删除行内的注释
        trim(line);
        if (line.size() > 0)
            // 不是空行，则进一步分析这一行脚本
            parseLine(line);
        lineCnt++;
    }
    inFile.close();
}

/// @brief 
/// @param line 
void Parser::parseLine(Line line) {
    //boost::split(tokenStream, line, boost::is_any_of(BLANKS), boost::token_compress_on);
    splitString(line, tokenStream, BLANKS);

    //TODO 需要仔细想想！！有没有什么好的方法？
    // 字符串中如果含有空白字符，也会被分割开，需要进行合并处理
    if (tokenStream[0] == "Out") {
        TokenStream tmpTokenStream;
        Token tmpConstStr = "";
        int cnt = 0; // 1 表示扫描到了一个双引号
        for (TokenStream::iterator iter = tokenStream.begin(); iter != tokenStream.end(); iter++) {

            // 匹配到了非转义字符的双引号
            if (std::regex_match(*iter, std::regex(".*[^\\]\".*|.*\\\\\".*"))) {
                tmpConstStr += *iter;
                // 已经扫描到一个双引号，则这个与之对应，形成一个字符串，加入 stream 并将变量清零
                if (cnt) {
                    tmpTokenStream.push_back(tmpConstStr);
                    tmpConstStr = "";
                    cnt = 0;
                }
                // 扫描到一个字符串的起始双引号，cnt 加一
                else cnt++;
            }
            // 当前步骤在字符串的扫描中，所有 token 都属于字符串
            else if (cnt) {
                tmpConstStr += *iter;
            }
            // 当前不在字符串的扫描中，且没有找到双引号，将 token 加入 stream
            else
                tmpTokenStream.push_back(*iter);
        }
    }
    procTokens();
}

//todo  可以使用设计模式进行改进

/// @brief 根据每行的第一个 token 确定下一步的动作
void Parser::procTokens() {
    Token lineKey = popFront(tokenStream);
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
    // 如果 Step 之后缺少 StepID，进入错误处理模块
    // 否则将当前步骤设置为新扫描到的步骤
    if ((curStepID = popFront(tokenStream)) == STREAM_EMPTY) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // StepID 不符合命名规范，进入错误处理模块
    if (LexAna(curStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // StepID 之后还有 Token，不符合语法，进入错误处理模块
    if (!tokenStream.empty()) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    // 当前步骤的标识已经出现过了，进入错误处理模块
    if (parseTree.stepTable.find(curStepID) != parseTree.stepTable.end()) {
        error(ERR_STATE::DuplicatedStep);
        return;
    }

    // 如果是第一个 Step，设置 entry 为当前 Step
    if (parseTree.stepTable.empty())
        parseTree.entry = curStepID;

    // 在步骤表中添加当前步骤
    parseTree.stepTable[curStepID] = StepActVec();
}


void Parser::procOut() {
    // Out 后面没有内容，进入错误处理模块
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // 检查 Out 后面的表达式，如果正确就逐个保存
    Expression expr;
    EXPR_STATE state = EXPR_STATE::WaitToken;
    Token item;
    while (!tokenStream.empty()) {
        item = popFront(tokenStream);

        // 当前项不符合命名规范，进入错误处理模块
        if (LexAna(item).checkLexical() == LexAna::TYPE::Err) {
            error(ERR_STATE::LexicalError);
            return;
        }
        // 符合命名规范，则进一步检查是否符合表达式的语法
        else
        {
            switch (state)
            {
            case Parser::EXPR_STATE::WaitAdd:
                if (item == "+") {
                    // 出现 + 号，但后面没有其他内容，进入错误处理模块
                    if (tokenStream.empty()) {
                        error(ERR_STATE::WrongExprssion);
                        return;
                    }
                    // 出现 + 号，后面还有内容，则等待出现 token
                    state = Parser::EXPR_STATE::WaitToken;
                    expr.push_back(item);
                }
                else
                {
                    // 没有出现 + 号，进入错误处理模块
                    error(ERR_STATE::UnknownToken);
                    return;
                }
                break;
            case Parser::EXPR_STATE::WaitToken:
                // 出现的 token 是变量或者字符串
                if (item.at(0) == '$' || item.at(0) == '"') {
                    // 加入变量表
                    if (item.at(0) == '$')
                        parseTree.vars.push_back(item);
                    expr.push_back(item);
                    // token 之后没有其他内容了，表达式正确，退出循环
                    if (tokenStream.empty())
                        break;
                    // token 之后还有其他内容，期待出现 + 号
                    state = Parser::EXPR_STATE::WaitAdd;
                }
                // 出现了未知的项目，进入错误处理模块
                else {
                    error(ERR_STATE::UnknownToken);
                    return;
                }
                break;
            default:
                break;
            }
        }
    }

    // 将要输出的表达式保存
    parseTree.stepTable[curStepID].push_back(std::make_unique<Out>(expr));
}

void Parser::procListen() {
    // Listen 后面没有内容，进入错误处理模块
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Listen 后面不止一个参数，进入错误处理模块
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }


    Token para = popFront(tokenStream);
    int listenTime = 0;
    try {
        listenTime = std::stoi(para);
    }
    // 参数类型不正确，进入错误处理模块
    catch (std::invalid_argument&) {
        error(ERR_STATE::WrongType);
        return;
    }

    // 保存到语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Listen>(listenTime));
}

void Parser::procBranch() {
    // Branch 参数个数小于 2，进入错误处理模块
    if (tokenStream.size() < 2) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Branch 后面不止 2 个参数，进入错误处理模块
    if (tokenStream.size() > 2) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token answer = popFront(tokenStream);
    // 参数 1 不是字符串，进入错误处理模块
    if (LexAna(answer).checkLexical() != LexAna::TYPE::ConstStr) {
        error(ERR_STATE::WrongType);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数 2 不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Branch>(answer, nextStepID));

    // 加入用户关键字表
    parseTree.keyWords.push_back(answer);
}


void Parser::procSilence() {
    // silence 后面没有内容，进入错误处理模块
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // silence 后的参数大于 1，进入错误处理模块
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Silence>(nextStepID));
}

void Parser::procDefault() {
    // default 后面没有内容，进入错误处理模块
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // default 后的参数大于 1，进入错误处理模块
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Default>(nextStepID));
}

void Parser::procExit() {
    parseTree.exits.push_back(curStepID);
}
