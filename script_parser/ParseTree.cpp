#include "ParseTree.h"
#include "Action.h"

ParseTree::~ParseTree() {
    // �ͷ� Action ָ��
    std::cout << "release Action pointers." << std::endl;
}