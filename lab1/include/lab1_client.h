#ifndef LAB1_CLIENT_H
#define LAB1_CLIENT_H

#include "lab1_message_code.h"

class client {
private:
    std::string username; // 用户名
    int client_fd; // 套接字
    std::mutex mtx; // 用于线程同步的互斥锁
    std::string selected_mode; // 选择的模式
    std::string target_name; // 私聊对象
    bool login_status; // 是否登录
    bool select_mode; // 是否选择聊天模式
    bool choose_partner; // 是否选择私聊对象
    bool quit_status; // 退出状态 

public:
    client();
    ~client();
    int init(); // 初始化
    int login(); // 用户登录
    int selectMode(); // 选择模式
    int choosePartner(); // 选择私聊对象
    int chat();
    int Send(std::string mode, char* message, std::string target_username, std::string chosen_mode); // 发送消息
    Message Recv();
    void serverRecv(); // 接受来自服务器端的消息
    std::string getUsername() { return username; }
};

#endif // LAB1_CLIENT_H