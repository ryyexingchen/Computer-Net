#include"lab1.h"
#include"lab1_client.h"
using namespace std;

int main() {
    client cli;
    if (cli.init() == 0) { // 初始化客户端并连接到服务器
        cli.chat(); // 用户选择聊天模式
    }
    return 0;
}