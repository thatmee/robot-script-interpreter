#include "pch.h"
#include "Interpreter/ScriptServer.h"
#include "Interpreter/User.h"

TEST(ScriptServer, createUser)
{
    ScriptServer s("./scripts/complete-1.rsl");

    EXPECT_TRUE(ScriptServer::NEW_USER_STA::Succeed == s.createUser("111"));
    EXPECT_TRUE(ScriptServer::NEW_USER_STA::WrongIDFormat == s.createUser("1111"));
    EXPECT_TRUE(ScriptServer::NEW_USER_STA::WrongIDFormat == s.createUser("abc"));
    EXPECT_TRUE(ScriptServer::NEW_USER_STA::AlreadyExists == s.createUser("111"));

}

TEST(ScriptServer, deleteUser)
{
    ScriptServer s("./scripts/complete-1.rsl");

    s.createUser("111");
    UserID id("222");
    EXPECT_TRUE(ScriptServer::DEL_USER_STA::NoSuchUser == s.deleteUser(id));
    id = "111";
    EXPECT_TRUE(ScriptServer::DEL_USER_STA::Succeed == s.deleteUser(id));
}

TEST(interpreter, interpret)
{
    Interpreter i("./scripts/complete-1.rsl");
    i.initParseTree();
    User user("111");
    user.curStepID = "welcome";
    user.vars = {
        {"$name","nyf"},
        {"$amount", "100"},
        {"$broadBand", "500"} };

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "nyf���ã�������ʲô���԰���?");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "����";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "������˼���Ҳ�̫��������˼�������Գ������¹ؼ��ʣ�Ͷ�ߡ��˵��������ά�ޡ��˹�");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "Ͷ��";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "��������������");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "question";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "������������ǸĽ������Ķ���������������ʲô����?");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "�˵�";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "���ı����˵��� 100 Ԫ");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "���ʻ���ʲô���԰�����");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "����";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "��������ʣ�� 500 MB");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "���ʻ���ʲô���԰�����");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "�˹�";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "���Ժ�����Ϊ�������˹��ͷ�");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "�ѽ���");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Exit);
    EXPECT_TRUE(user.finished == false);
}


TEST(ScriptServer, initCorrect)
{
    ScriptServer s("./scripts/complete-1.rsl");

    s.init();

    std::ifstream sampleLog, realLog;
    sampleLog.open("./log/sample_log/complete-1.log");
    realLog.open("./log/complete-1.log");
    std::string sampleStr, realStr;
    while (!sampleLog.eof())
        sampleStr += sampleLog.get();
    while (!realLog.eof())
        realStr += realLog.get();
    sampleLog.close();
    realLog.close();
    EXPECT_TRUE(sampleStr == realStr);
}

TEST(ScriptServer, initWrong)
{
    ScriptServer s("./scripts/incomplete.rsl");

    s.init();

    std::ifstream sampleLog, realLog;
    sampleLog.open("./log/sample_log/incomplete.log");
    realLog.open("./log/incomplete.log");
    std::string sampleStr, realStr;
    while (!sampleLog.eof())
        sampleStr += sampleLog.get();
    while (!realLog.eof())
        realStr += realLog.get();
    sampleLog.close();
    realLog.close();
    EXPECT_TRUE(sampleStr == realStr);
}