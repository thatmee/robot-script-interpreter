#include "ParseTree.h"
#include "Action.hpp"

ParseTree::~ParseTree() {
    // �ͷ� Action ָ��
    std::cout << "release Action pointers." << std::endl;
}