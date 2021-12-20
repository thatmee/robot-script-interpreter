#include "Parser.h"

Parser::~Parser() {}


Parser::Parser(const char* scriptPath_)
    :scriptPath(scriptPath_), parseTree(), lineCnt(1) {}


void Parser::error(ERR_STATE err)
{
    everythingRight = false;
    dbg.out("line " + std::to_string(lineCnt) + "\terror" + std::string(magic_enum::enum_name(err)) + "\n", logName);
}


void Parser::error(ERR_STATE err, std::string msg)
{
    everythingRight = false;
    dbg.out("error: " + std::string(magic_enum::enum_name(err)) + "\t" + msg + "\n", logName);
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

    // ɾ��ע�ͣ���������ע�ͺ�����ע��
    // һ��Ҫ��ɾ��ע�ͣ���ɾ���հ��ַ�������հ��ַ���������©��������һ���: "Listen 20     #�ȴ�20s"
    std::string::size_type commentPos = str.find_first_of(ANNOTATION_SYMBOL);
    if (commentPos != std::string::npos)
        str.erase(commentPos);

    // ɾ�����׿հ��ַ�
    str.erase(0, str.find_first_not_of(BLANKS));

    // ɾ����β�հ��ַ�
    std::string::size_type lastNotBlank = str.find_last_not_of(BLANKS);
    std::string::size_type lastBlank = str.find_last_of(BLANKS);
    // lastNotBlank < lastBlank �����ų�ĩβû�пհ��ַ������
    // ĩβû�пհ��ַ�ʱ�����һ���հ��ַ� lastBlank һ�������һ���ǿ��ַ� lastNotBlank ��ǰ��
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
    // pos2 ָ���Ӵ���β����ʼΪ��һ���ָ�����λ��
    pos2 = s.find_first_of(delim);
    // pos1 ָ���Ӵ����ף���Ϊ split ǰ�Ѿ�ȥ���˿հ��ַ������� pos1 ��ʼΪ 0
    pos1 = 0;

    // ѭ���ָ��ַ���
    while (std::string::npos != pos2)
    {
        std::string tmp = s.substr(pos1, pos2 - pos1);
        if (!tmp.empty())
        {
            // ���Ӵ����׵�β������Ϊ�գ��򹹳�һ���ָ������� result
            result.push_back(tmp);
        }
        pos1 = s.find_first_not_of(delim, pos2 + 1);
        pos2 = s.find_first_of(delim, pos1);
    }
    // ��ʣ���һ���Ӵ����� result
    if (pos1 != s.length())
        result.push_back(s.substr(pos1));
}


void Parser::parseFile()
{
    std::ifstream inFile;
    inFile.open(scriptPath);

    // �ļ���ʧ��
    if (!inFile.is_open())
    {
        dbg.out("fail to open script.\n", logName);
        everythingRight = false;
        return;
    }

    // �ļ��򿪳ɹ������ж�ȡ�ű��ļ����з���
    while (!inFile.eof())
    {
        char lineBuf[LINE_BUFFER_SIZE];
        inFile.getline(lineBuf, LINE_BUFFER_SIZE);
        Line line = lineBuf;
        // ɾ�����׺���β�Ŀհ��ַ���ɾ�����ڵ�ע��
        trim(line);
        if (line.size() > 0)
            // ���ǿ��У����һ��������һ�нű�
            parseLine(line);
        lineCnt++;
    }

    // ���з�����ϣ����﷨������������
    checkTree();
    inFile.close();
}


void Parser::parseLine(Line line)
{
    tokenStream.clear();
    splitString(line, tokenStream, BLANKS);

    // �ַ�����������пհ��ַ���Ҳ�ᱻ�ָ����Ҫ���кϲ�����
    if (tokenStream[0] == "Out" || tokenStream[0] == "Branch")
    {
        TokenStream tmpTokenStream;
        Token tmpConstStr = "";
        int cnt = 0; // 1 ��ʾɨ�赽��һ��˫���ţ�0 Ϊû��ɨ�赽˫����
        for (TokenStream::iterator iter = tokenStream.begin(); iter != tokenStream.end(); iter++)
        {
            // ��ǰ Token ����һ�Է�ת���˫����
            if (std::regex_match(*iter, std::regex(R"(^([^\\]|\\.)*"([^\\]|\\.)*".*$)")))
            {
                // ��� cnt Ϊ 1������������ "abc""def" �Ĵ������������ģ��
                if (cnt)
                {
                    error(Parser::ERR_STATE::LexicalError);
                    return;
                }
                // ����ֱ�ӽ���ǰ Token ���� tmpTokenStream
                else
                    tmpTokenStream.push_back(*iter);
            }

            // ��ǰ Token ��ֻ��һ����ת���ַ���˫����
            else if (std::regex_match(*iter, std::regex(R"(([^\\]|\\.)*".*)")))
            {
                // �Ѿ�ɨ�赽һ��˫���ţ��������֮��Ӧ���γ�һ���ַ��������� stream ������������
                if (cnt)
                {
                    tmpConstStr += " ";
                    tmpConstStr += *iter;
                    tmpTokenStream.push_back(tmpConstStr);
                    tmpConstStr = "";
                    cnt = 0;
                }
                // ɨ�赽һ���ַ�������ʼ˫���ţ�cnt ��һ
                else
                {
                    tmpConstStr += *iter;
                    cnt++;
                }
            }

            // ��ǰ Token ��û��˫����
            // ���� cnt Ϊ 1����ǰ���账���ַ�����ɨ���У��� token �����ַ���
            else if (cnt)
            {
                tmpConstStr += " ";
                tmpConstStr += *iter;
            }

            // ��ǰ Token ��û��˫���ţ��ҵ�ǰ�����ַ�����ɨ����
            // �� token ���� tmpTokenStream
            else
                tmpTokenStream.push_back(*iter);
        }
        tokenStream = tmpTokenStream;
    }
    procTokens();
}


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
    // ��� Step ֮��ȱ�� StepID�����������ģ��
    // ���򽫵�ǰ��������Ϊ��ɨ�赽�Ĳ���
    if ((curStepID = popFront(tokenStream)) == STREAM_EMPTY)
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // StepID �����������淶�����������ģ��
    if (LexAna(curStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // StepID ֮���� Token���������﷨�����������ģ��
    if (!tokenStream.empty())
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    // ��ǰ����ı�ʶ�Ѿ����ֹ��ˣ����������ģ��
    if (parseTree.stepTable.find(curStepID) != parseTree.stepTable.end())
    {
        error(ERR_STATE::DuplicatedStep);
        return;
    }

    // ����ǵ�һ�� Step������ entry Ϊ��ǰ Step
    if (parseTree.stepTable.empty())
        parseTree.entry = curStepID;

    // �ڲ��������ӵ�ǰ����
    parseTree.stepTable[curStepID] = StepActVec();
}


void Parser::procOut()
{
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Out ����û�����ݣ����������ģ��
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // ��� Out ����ı��ʽ�������ȷ���������
    Expression expr;
    EXPR_STATE state = EXPR_STATE::WaitToken;
    Token item;
    while (!tokenStream.empty())
    {
        item = popFront(tokenStream);

        // ��ǰ����������淶�����������ģ��
        if (LexAna(item).checkLexical() == LexAna::TYPE::Err)
        {
            error(ERR_STATE::LexicalError);
            return;
        }

        // ���������淶�����һ������Ƿ���ϱ��ʽ���﷨
        else
        {
            switch (state)
            {
            case Parser::EXPR_STATE::WaitAdd:
                if (item == "+")
                {
                    // ���� + �ţ�������û���������ݣ����������ģ��
                    if (tokenStream.empty())
                    {
                        error(ERR_STATE::WrongExprssion);
                        return;
                    }
                    // ���� + �ţ����滹�����ݣ���ȴ����� token
                    state = Parser::EXPR_STATE::WaitToken;
                    // + �Ų���ӵ��﷨��
                    //expr.push_back(item);
                }
                else
                {
                    // û�г��� + �ţ����������ģ��
                    error(ERR_STATE::WrongExprssion);
                    return;
                }
                break;
            case Parser::EXPR_STATE::WaitToken:
                if (item.at(0) == '$')
                {
                    // ���ֵ� token �Ǳ���
                    if (std::find(parseTree.vars.begin(), parseTree.vars.end(), item) == parseTree.vars.end())
                    {
                        // token ���ڱ������У����������
                        parseTree.vars.push_back(item);
                    }
                    // ����������ʽ
                    expr.push_back(item);
                    // token ֮��û�����������ˣ����ʽ��ȷ���˳�ѭ��
                    if (tokenStream.empty())
                        break;
                    // token ֮�����������ݣ��ڴ����� + ��
                    state = Parser::EXPR_STATE::WaitAdd;
                }
                else if (item.at(0) == '"')
                {
                    // ���ֵ� token ���ַ���
                    // �ַ���ȥ��˫����
                    item.erase(0, 1);
                    item.pop_back();
                    // �ַ���ȥ��ת���ַ�
                    std::match_results;
                    Item tmp = "";
                    std::regex pattern(R"(\\(.))");
                    auto words_begin = std::sregex_iterator(item.begin(), item.end(), pattern);
                    auto words_end = std::sregex_iterator();
                    std::sregex_iterator i;
                    // pos ���ڱ������һ��ƥ�䵽�� match ��λ�ã�����������һ�� match �ĺ�׺
                    int pos = -2;
                    for (i = words_begin; i != words_end; i++)
                    {
                        std::smatch match = *i;
                        std::string match_str = match.str();
                        tmp += match.prefix();
                        tmp += match_str[1];
                        pos = match.position();
                    }
                    tmp += item.substr(pos + 2);
                    expr.push_back(tmp);
                    // token ֮��û�����������ˣ����ʽ��ȷ���˳�ѭ��
                    if (tokenStream.empty())
                        break;
                    // token ֮�����������ݣ��ڴ����� + ��
                    state = Parser::EXPR_STATE::WaitAdd;

                }

                else
                {
                    // ������δ֪����Ŀ�����������ģ��
                    error(ERR_STATE::WrongExprssion);
                    return;
                }
                break;
            default:
                break;
            }
        }
    }

    // ��Ҫ����ı��ʽ����
    parseTree.stepTable[curStepID].push_back(std::make_unique<Out>(expr));
}


void Parser::procListen()
{
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Listen ����û�����ݣ����������ģ��
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Listen ���治ֹһ�����������������ģ��
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
    // �������Ͳ���ȷ�����������ģ��
    catch (std::invalid_argument&)
    {
        error(ERR_STATE::WrongType);
        return;
    }

    // ���浽�﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Listen>(listenTime));
}


void Parser::procBranch()
{
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Branch ��������С�� 2�����������ģ��
    if (tokenStream.size() < 2)
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Branch ���治ֹ 2 �����������������ģ��
    if (tokenStream.size() > 2)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token answer = popFront(tokenStream);
    // ���� 1 �����ַ��������������ģ��
    if (LexAna(answer).checkLexical() != LexAna::TYPE::ConstStr)
    {
        error(ERR_STATE::WrongType);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // ���� 2 ���Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // ���� 1 ȥ��˫����
    answer.erase(0, 1);
    answer.pop_back();

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Branch>(answer, nextStepID));

    // ����ؼ��ֲ����ڣ������û��ؼ��ֱ�
    if (std::find(parseTree.keyWords.begin(), parseTree.keyWords.end(), answer) == parseTree.keyWords.end())
        parseTree.keyWords.push_back(answer);
}


void Parser::procSilence()
{
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // silence ����û�����ݣ����������ģ��
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // silence ��Ĳ������� 1 �������������ģ��
    if (tokenStream.size() > 1)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // �������Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Silence>(nextStepID));
}


void Parser::procDefault() {
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // default ����û�����ݣ����������ģ��
    if (tokenStream.empty())
    {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // default ��Ĳ������� 1�����������ģ��
    if (tokenStream.size() > 1)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // �������Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier)
    {
        error(ERR_STATE::LexicalError);
        return;
    }

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Default>(nextStepID));
}


void Parser::procExit()
{
    // ��ǰ����û�ж�Ӧ�� Step�����������ģ��
    if (curStepID == "")
    {
        error(Parser::ERR_STATE::NoCorrespondingStep);
        return;
    }

    // Exit �����������ݣ����������ģ��
    if (tokenStream.size() > 0)
    {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    parseTree.exits.push_back(curStepID);

    // Exit ֮�󣬵�ǰ���費���ٳ�����������������ǰ������Ϊ��
    curStepID = "";
}


bool Parser::generateParseTree()
{
#ifdef GTEST

    dbg.setDbgLevel(DBG::DBG_LEVELS::Test);
    dbg.setOutPipe(DBG::PIPES::FileIO);
    dbg.anaLogName(scriptPath, logName);
    // ɾ��ԭ���ļ�
    std::ofstream log(logName);
    if (log.is_open())
    {
        log << "";
        log.close();
    }

#else
    dbg.setDbgLevel(DBG::DBG_LEVELS::Simple);
    dbg.setOutPipe(DBG::PIPES::Standard);
#endif // GTEST

    dbg.out("generateParseTree...\n", logName);

    parseFile();

    // ����ʱ����﷨��
    parseTree.output(logName);

    if (everythingRight)
        dbg.out("parse file finished.\n", logName);
    else
        dbg.out("parse file failed.\n", logName);

    // ���ط������
    return everythingRight;
}


void Parser::checkTree()
{
    // �����﷨��
    for (StepTable::iterator tIter = parseTree.stepTable.begin(); tIter != parseTree.stepTable.end(); tIter++)
    {
        for (StepActVec::iterator vIter = tIter->second.begin(); vIter != tIter->second.end(); vIter++)
        {
            Action::ActionType type = (*vIter)->getCurType();
            StepID nextStepID;

            // ��������� Branch��Silence��Default �е�һ�֣���ȡ�� nextStepID
            if (type == Action::ActionType::Branch)
            {
                Branch* branch = static_cast<Branch*>(vIter->get());
                branch->getNextStepID(nextStepID);
            }
            else if (type == Action::ActionType::Silence)
            {
                Silence* silence = static_cast<Silence*>(vIter->get());
                silence->getNextStepID(nextStepID);
            }
            else if (type == Action::ActionType::Default)
            {
                Default* d = static_cast<Default*>(vIter->get());
                d->getNextStepID(nextStepID);
            }
            // ������������͵Ķ�����ֱ������
            else
                continue;

            // ��ȡ���� StepID ���� StepTable �У����������ģ��
            if (parseTree.stepTable.find(nextStepID) == parseTree.stepTable.end())
                error(Parser::ERR_STATE::UnknownToken, "An undefined stepID: " + nextStepID);
        }
}
}

