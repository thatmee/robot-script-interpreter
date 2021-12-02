#include "ScriptServer.h"

ScriptServer::ScriptServer(const char* scriptName_) {
    this->scriptName = scriptName_;
    parser.generateParseTree(parseTree);
}
