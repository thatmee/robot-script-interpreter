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
    EXPECT_TRUE(user.outputMsg == "nyf您好，请问有什么可以帮您?");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "您好";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "不好意思，我不太懂您的意思，您可以尝试以下关键词：投诉、账单、宽带、维修、人工");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "投诉";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "请描述您的问题");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "question";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "您的意见是我们改进工作的动力，请问您还有什么补充?");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "账单";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "您的本月账单是 100 元");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "请问还有什么可以帮您？");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "流量";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "您的流量剩余 500 MB");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "请问还有什么可以帮您？");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;

    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Listen);
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    user.inputKey = "人工";
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "请稍后，正在为您接入人工客服");
    EXPECT_TRUE(user.finished == false);
    user.finished = true;
    EXPECT_TRUE(i.interpret(user) == Interpreter::STA::Out);
    EXPECT_TRUE(user.outputMsg == "已接入");
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