#include "ScriptServer.h"

void ScriptServer::createUser(UserID ID_, UserName name_ = "") {
    // ������ʽ�ж� ID �Ƿ���Ϲ��򣬲���������������ģ��
    // error(ScriptServer::ERR_STA::WrongID);
    // return;

    User user(ID_, name_);

    // ���� Interpreter������ parser tree ��ʼ���û�����
    interpreter.initExecEnv(user);

    // ��ȡ���������ݣ�����û��ı�����
    // std::cin;
    // ���ݿ⣿��

    // ���� userTable
    // �ǵ� make_unique
}



