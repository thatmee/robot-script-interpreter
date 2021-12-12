#include "Parser.h"


Parser::Parser(const char* scriptPath_)
    :scriptPath(scriptPath_), parseTree(), lineCnt(1)
{
#ifdef GTEST
    // 从脚本文件路径获取当前脚本文件的名称
    std::string pathStr(scriptPath);
    std::string scriptName;
    std::string::size_type sep = pathStr.find_last_of("/");
    std::string::size_type suf = pathStr.find_last_of(".");
    if (sep == std::string::npos)
        sep = 0;
    if (suf == std::string::npos)
        scriptName = pathStr.substr(sep);
    else if (sep < suf)
        scriptName = pathStr.substr(sep, suf);
    else
        scriptName = "error";

    // 测试日志文件名称为：脚本文件名称.log
    std::string logName = "./log/" + scriptName + ".log";

    // 打开测试日志文件
    logFile.open(logName);
    if (!logFile.is_open())
        std::cout << "fail to open error log." << std::endl;
#endif // GTEST
}


void Parser::error(ERR_STATE err)
{
#ifdef GTEST
    logFile << "line " << lineCnt << "\terror:" << magic_enum::enum_name(err) << std::endl;

#else
    // 处理错误
    std::cout << "error:" << magic_enum::enum_name(err) << std::endl;
#endif // GTEST
}


Token Parser::popFront(TokenStream& stream)
{
    if (stream.empty())
        return STREAM_EMPTY;
    Token front = stream.at(0);
    stream.erase(stream.begin());
    return front;
}


void Parser::trim(std::string& str)
{
    if (str.empty()) return;

    // 删除注释，包括单行注释和行内注释
    // 一定要先删除注释，再删除空白字符，否则空白字符可能有遗漏，比如这一语句: "Listen 20     #等待20s"
    std::string::size_type commentPos = str.find_first_of(ANNOTATION_SYMBOL);
    if (commentPos != std::string::npos)
        str.erase(commentPos);

    // 删除行首空白字符
    str.erase(0, str.find_first_not_of(BLANKS));

    // 删除行尾空白字符
    std::string::size_type lastNotBlank = str.find_last_not_of(BLANKS);
    std::string::size_type lastBlank = str.find_last_of(BLANKS);
    // lastNotBlank < lastBlank 用于排除末尾没有空白字符的情况
    // 末尾没有空白字符时，最后一个空白字符 lastBlank 一定在最后一个非空字符 lastNotBlank 的前面
    if (lastNotBlank != std::string::npos && lastBlank != std::string::npos
        && lastNotBlank < lastBlank)
        str.erase(lastNotBlank + 1);
}


void Parser::splitString
(
    const std::string& s,
    std::vector<std::string>& result,
    const std::string& delim)
{
    std::string::size_type pos1, pos2;
    // pos2 指向子串的尾，初始为第一个分隔符的位置
    pos2 = s.find_first_of(delim);
    // pos1 指向子串的首，因为 split 前已经去除了空白字符，所以 pos1 起始为 0
    pos1 = 0;

    // 循环分割字符串
    while (std::string::npos != pos2)
    {
        std::string tmp = s.substr(pos1, pos2 - pos1);
        if (!tmp.empty())
        {
            // 从子串的首到尾，若不为空，则构成一个分割并将其加入 result
            result.push_back(tmp);
        }
        pos1 = s.find_first_not_of(delim, pos2 + 1);
        pos2 = s.find_first_of(delim, pos1);
    }
    // 将剩余的一个子串加入 result
    if (pos1 != s.length())
        result.push_back(s.substr(pos1));
}


void Parser::parseFile()
{
    std::ifstream inFile;
    inFile.open(scriptPath);
    // 文件打开失败
    if (!inFile.is_open())
    {
        std::cout << "fail to open script." << std::endl;
        return;
    }
    // 文件打开成功，逐行读取脚本文件
    while (!inFile.eof())
    {
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


void Parser::parseLine(Line line)
{
    tokenStream.clear();
    splitString(line, tokenStream, BLANKS);

    //TODO 需要仔细想想！！有没有什么好的方法？
    // 字符串中如果含有空白字符，也会被分割开，需要进行合并处理
    if (tokenStream[0] == "Out" || tokenStream[0] == "Branch")
    {
        TokenStream tmpTokenStream;
        Token tmpConstStr = "";
        int cnt = 0; // 1 表示扫描到了一个双引号，0 为没有扫描到双引号
        for (TokenStream::iterator iter = tokenStream.begin(); iter != tokenStream.end(); iter++)
        {
            // 当前 Token 含有一对非转义的双引号
            if (std::regex_match(*iter, std::regex(R"(^([^\\]|\\.)*"([^\\]|\\.)*".*$)")))
            {
                // 如果 cnt 为 1，出现了形如 "abc""def" 的串，进入错误处理模块
                if (cnt)
                {
                    error(Parser::ERR_STATE::LexicalError);
                    return;
                }
                // 否则，直接将当前 Token 加入 tmpTokenStream
                else
                    tmpTokenStream.push_back(*iter);
            }

            // 当前 Token 中只有一个非转义字符的双引号
            else if (std::regex_match(*iter, std::regex(R"(([^\\]|\\.)*".*)")))
            {
                // 已经扫描到一个双引号，则这个与之对应，形成一个字符串，加入 stream 并将变量清零
                if (cnt)
                {
                    tmpConstStr += " ";
                    tmpConstStr += *iter;
                    tmpTokenStream.push_back(tmpConstStr);
                    tmpConstStr = "";
                    cnt = 0;
                }
                // 扫描到一个字符串的起始双引号，cnt 加一
                else
                {
                    tmpConstStr += *iter;
                    cnt++;
                }
            }

            // 当前 Token 中没有双引号
            // 但是 cnt 为 1，当前步骤处于字符串的扫描中，将 token 加入字符串
            else if (cnt)
            {
                tmpConstStr += " ";
                tmpConstStr += *iter;
            }

            // 当前 Token 中没有双引号，且当前不在字符串的扫描中
            // 将 token 加入 tmpTokenStream
            else
                tmpTokenStream.push_back(*iter);
        }
        tokenStream = tmpTokenStream;
    }
    procTokens();
}

//todo  可以使用设计模式进行改进

void Parser::procTokens()
{
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


void Parser::procStep()
{
    // 如果 Step 之后缺少 StepID，进入错误处理模块
    // 否则将当前步骤设置为新扫描到的步骤
    if ((curStepID = popFront(tokenStream)) == STREAM_EMPTY)
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // StepID 不符合命名规范，进入错误处理模块
    if (LexAna(curStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // StepID 之后还有 Token，不符合语法，进入错误处理模块
    if (!tokenStream.empty())
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    // 当前步骤的标识已经出现过了，进入错误处理模块
    if (parseTree.stepTable.find(curStepID) != parseTree.stepTable.end())
    {
        error(ERR_STATE::DuplicatedStep);
        return;
    }

    // 如果是第一个 Step，设置 entry 为当前 Step
    if (parseTree.stepTable.empty())
        parseTree.entry = curStepID;

    // 在步骤表中添加当前步骤
    parseTree.stepTable[curStepID] = StepActVec();
}


void Parser::procOut()
{
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Out 后面没有内容，进入错误处理模块
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // 检查 Out 后面的表达式，如果正确就逐个保存
    Expression expr;
    EXPR_STATE state = EXPR_STATE::WaitToken;
    Token item;
    while (!tokenStream.empty())
    {
        item = popFront(tokenStream);

        // 当前项不符合命名规范，进入错误处理模块
        if (LexAna(item).checkLexical() == LexAna::TYPE::Err)
        {
            error(ERR_STATE::LexicalError);
            return;
        }

        // 符合命名规范，则进一步检查是否符合表达式的语法
        else
        {
            switch (state)
            {
            case Parser::EXPR_STATE::WaitAdd:
                if (item == "+")
                {
                    // 出现 + 号，但后面没有其他内容，进入错误处理模块
                    if (tokenStream.empty())
                    {
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
                    error(ERR_STATE::WrongExprssion);
                    return;
                }
                break;
            case Parser::EXPR_STATE::WaitToken:
                // 出现的 token 是变量或者字符串
                if (item.at(0) == '$' || item.at(0) == '"')
                {
                    // 出现的 token 是变量，且不在表中，加入变量表
                    if (item.at(0) == '$'
                        && (std::find(parseTree.vars.begin(), parseTree.vars.end(), item) == parseTree.vars.end()))
                        parseTree.vars.push_back(item);
                    // 变量或字符串都加入表达式
                    expr.push_back(item);
                    // token 之后没有其他内容了，表达式正确，退出循环
                    if (tokenStream.empty())
                        break;
                    // token 之后还有其他内容，期待出现 + 号
                    state = Parser::EXPR_STATE::WaitAdd;
                }
                // 出现了未知的项目，进入错误处理模块
                else
                {
                    error(ERR_STATE::WrongExprssion);
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

void Parser::procListen()
{
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Listen 后面没有内容，进入错误处理模块
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Listen 后面不止一个参数，进入错误处理模块
    if (tokenStream.size() > 1)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }


    Token para = popFront(tokenStream);
    int listenTime = 0;
    try
    {
        listenTime = std::stoi(para);
    }
    // 参数类型不正确，进入错误处理模块
    catch (std::invalid_argument&)
    {
        error(ERR_STATE::WrongType);
        return;
    }

    // 保存到语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Listen>(listenTime));
}

void Parser::procBranch()
{
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Branch 参数个数小于 2，进入错误处理模块
    if (tokenStream.size() < 2)
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Branch 后面不止 2 个参数，进入错误处理模块
    if (tokenStream.size() > 2)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token answer = popFront(tokenStream);
    // 参数 1 不是字符串，进入错误处理模块
    if (LexAna(answer).checkLexical() != LexAna::TYPE::ConstStr)
    {
        error(ERR_STATE::WrongType);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数 2 不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Branch>(answer, nextStepID));

    // 如果关键字不存在，加入用户关键字表
    if (std::find(parseTree.keyWords.begin(), parseTree.keyWords.end(), answer) == parseTree.keyWords.end())
        parseTree.keyWords.push_back(answer);
}


void Parser::procSilence()
{
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // silence 后面没有内容，进入错误处理模块
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // silence 后的参数大于 1 个，进入错误处理模块
    if (tokenStream.size() > 1)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Silence>(nextStepID));
}

void Parser::procDefault() {
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // default 后面没有内容，进入错误处理模块
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // default 后的参数大于 1，进入错误处理模块
    if (tokenStream.size() > 1)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // 参数不是标识符，进入错误处理模块
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // 加入语法树
    parseTree.stepTable[curStepID].push_back(std::make_unique<Default>(nextStepID));
}

void Parser::procExit()
{
    // 当前动作没有对应的 Step，进入错误处理模块
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Exit 后有其他内容，进入错误处理模块
    if (tokenStream.size() > 0)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    parseTree.exits.push_back(curStepID);

    // Exit 之后，当前步骤不能再出现其他动作，将当前步骤置为空
    curStepID = "";
}


void Parser::generateParseTree()
{
    std::cout << "generateParseTree" << std::endl;
    parseFile();
    std::cout << "parse file finished." << std::endl;
}