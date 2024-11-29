#include "WordQuery.h"
#include "WordQueryServer.h"
#include "mylogger.h"
using std::stoi;
using namespace wd;

int main() {
    Configuration::getInstance(
        "/home/wjj/桌面/search/SearchEnigine/conf/online.conf");

    WordQueryServer server(stoi(CONFIG["threadNum"]), stoi(CONFIG["queSize"]),
                           CONFIG["ip"], stoi(CONFIG["port"]));

    server.start();

    Mylogger::destroy();
    return 0;
}
