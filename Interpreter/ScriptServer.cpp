#include "ScriptServer.h"

void ScriptServer::createUser(UserID ID_, UserName name_ = "") {
    // 正则表达式判断 ID 是否符合规则，不符合则进入错误处理模块
    // error(ScriptServer::ERR_STA::WrongID);
    // return;

    User user(ID_, name_);

    // 调用 Interpreter，根据 parser tree 初始化用户数据
    interpreter.initExecEnv(user);

    // 读取服务器数据，填充用户的变量表
    // std::cin;
    // 数据库？？

    // 加入 userTable
    // 记得 make_unique
}



