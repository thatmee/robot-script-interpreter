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

/// @brief ɾ���ַ��� str ��β�Ŀհ��ַ���\f \v \r \t ��
///        ɾ��ע�ͣ���������ע�ͺ�����ע��
/// @param str 
void Parser::trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(BLANKS));  // ɾ�����׿հ��ַ�
    str.erase(str.find_last_not_of(BLANKS) + 1);  // ɾ����β�հ��ַ�
    str.erase(str.find_last_not_of(ANNOTATION_SYMBOL) + 1); //ɾ��ע�ͣ���������ע�ͺ�����ע��
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

/// @brief �Խű��ļ������﷨����������򿪡��ر��ļ������ļ�����
void Parser::parseFile() {
    std::ifstream inFile;
    inFile.open(scriptName);
    // ���ж�ȡ�ű��ļ�
    while (!inFile.eof()) {
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
    inFile.close();
}

/// @brief 
/// @param line 
void Parser::parseLine(Line line) {
    //boost::split(tokenStream, line, boost::is_any_of(BLANKS), boost::token_compress_on);
    splitString(line, tokenStream, BLANKS);

    //TODO ��Ҫ��ϸ���룡����û��ʲô�õķ�����
    // �ַ�����������пհ��ַ���Ҳ�ᱻ�ָ����Ҫ���кϲ�����
    if (tokenStream[0] == "Out") {
        TokenStream tmpTokenStream;
        Token tmpConstStr = "";
        int cnt = 0; // 1 ��ʾɨ�赽��һ��˫����
        for (TokenStream::iterator iter = tokenStream.begin(); iter != tokenStream.end(); iter++) {

            // ƥ�䵽�˷�ת���ַ���˫����
            if (std::regex_match(*iter, std::regex(".*[^\\]\".*|.*\\\\\".*"))) {
                tmpConstStr += *iter;
                // �Ѿ�ɨ�赽һ��˫���ţ��������֮��Ӧ���γ�һ���ַ��������� stream ������������
                if (cnt) {
                    tmpTokenStream.push_back(tmpConstStr);
                    tmpConstStr = "";
                    cnt = 0;
                }
                // ɨ�赽һ���ַ�������ʼ˫���ţ�cnt ��һ
                else cnt++;
            }
            // ��ǰ�������ַ�����ɨ���У����� token �������ַ���
            else if (cnt) {
                tmpConstStr += *iter;
            }
            // ��ǰ�����ַ�����ɨ���У���û���ҵ�˫���ţ��� token ���� stream
            else
                tmpTokenStream.push_back(*iter);
        }
    }
    procTokens();
}

//todo  ����ʹ�����ģʽ���иĽ�

/// @brief ����ÿ�еĵ�һ�� token ȷ����һ���Ķ���
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
    // ��� Step ֮��ȱ�� StepID�����������ģ��
    // ���򽫵�ǰ��������Ϊ��ɨ�赽�Ĳ���
    if ((curStepID = popFront(tokenStream)) == STREAM_EMPTY) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // StepID �����������淶�����������ģ��
    if (LexAna(curStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // StepID ֮���� Token���������﷨�����������ģ��
    if (!tokenStream.empty()) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    // ��ǰ����ı�ʶ�Ѿ����ֹ��ˣ����������ģ��
    if (parseTree.stepTable.find(curStepID) != parseTree.stepTable.end()) {
        error(ERR_STATE::DuplicatedStep);
        return;
    }

    // ����ǵ�һ�� Step������ entry Ϊ��ǰ Step
    if (parseTree.stepTable.empty())
        parseTree.entry = curStepID;

    // �ڲ��������ӵ�ǰ����
    parseTree.stepTable[curStepID] = StepActVec();
}


void Parser::procOut() {
    // Out ����û�����ݣ����������ģ��
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // ��� Out ����ı��ʽ�������ȷ���������
    Expression expr;
    EXPR_STATE state = EXPR_STATE::WaitToken;
    Token item;
    while (!tokenStream.empty()) {
        item = popFront(tokenStream);

        // ��ǰ����������淶�����������ģ��
        if (LexAna(item).checkLexical() == LexAna::TYPE::Err) {
            error(ERR_STATE::LexicalError);
            return;
        }
        // ���������淶�����һ������Ƿ���ϱ��ʽ���﷨
        else
        {
            switch (state)
            {
            case Parser::EXPR_STATE::WaitAdd:
                if (item == "+") {
                    // ���� + �ţ�������û���������ݣ����������ģ��
                    if (tokenStream.empty()) {
                        error(ERR_STATE::WrongExprssion);
                        return;
                    }
                    // ���� + �ţ����滹�����ݣ���ȴ����� token
                    state = Parser::EXPR_STATE::WaitToken;
                    expr.push_back(item);
                }
                else
                {
                    // û�г��� + �ţ����������ģ��
                    error(ERR_STATE::UnknownToken);
                    return;
                }
                break;
            case Parser::EXPR_STATE::WaitToken:
                // ���ֵ� token �Ǳ��������ַ���
                if (item.at(0) == '$' || item.at(0) == '"') {
                    // ���������
                    if (item.at(0) == '$')
                        parseTree.vars.push_back(item);
                    expr.push_back(item);
                    // token ֮��û�����������ˣ����ʽ��ȷ���˳�ѭ��
                    if (tokenStream.empty())
                        break;
                    // token ֮�����������ݣ��ڴ����� + ��
                    state = Parser::EXPR_STATE::WaitAdd;
                }
                // ������δ֪����Ŀ�����������ģ��
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

    // ��Ҫ����ı��ʽ����
    parseTree.stepTable[curStepID].push_back(std::make_unique<Out>(expr));
}

void Parser::procListen() {
    // Listen ����û�����ݣ����������ģ��
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Listen ���治ֹһ�����������������ģ��
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }


    Token para = popFront(tokenStream);
    int listenTime = 0;
    try {
        listenTime = std::stoi(para);
    }
    // �������Ͳ���ȷ�����������ģ��
    catch (std::invalid_argument&) {
        error(ERR_STATE::WrongType);
        return;
    }

    // ���浽�﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Listen>(listenTime));
}

void Parser::procBranch() {
    // Branch ��������С�� 2�����������ģ��
    if (tokenStream.size() < 2) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // Branch ���治ֹ 2 �����������������ģ��
    if (tokenStream.size() > 2) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token answer = popFront(tokenStream);
    // ���� 1 �����ַ��������������ģ��
    if (LexAna(answer).checkLexical() != LexAna::TYPE::ConstStr) {
        error(ERR_STATE::WrongType);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // ���� 2 ���Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Branch>(answer, nextStepID));

    // �����û��ؼ��ֱ�
    parseTree.keyWords.push_back(answer);
}


void Parser::procSilence() {
    // silence ����û�����ݣ����������ģ��
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // silence ��Ĳ������� 1�����������ģ��
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // �������Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Silence>(nextStepID));
}

void Parser::procDefault() {
    // default ����û�����ݣ����������ģ��
    if (tokenStream.empty()) {
        error(ERR_STATE::TooFewTokens);
        return;
    }

    // default ��Ĳ������� 1�����������ģ��
    if (tokenStream.size() > 1) {
        error(ERR_STATE::TooManyTokens);
        return;
    }

    Token nextStepID = popFront(tokenStream);
    // �������Ǳ�ʶ�������������ģ��
    if (LexAna(nextStepID).checkLexical() != LexAna::TYPE::Identifier) {
        error(ERR_STATE::LexicalError);
        return;
    }

    // �����﷨��
    parseTree.stepTable[curStepID].push_back(std::make_unique<Default>(nextStepID));
}

void Parser::procExit() {
    parseTree.exits.push_back(curStepID);
}
