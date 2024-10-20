#ifndef LAB1_SERVER_H
#define LAB1_SERVER_H

#include "lab1_message_code.h"

class server {
private:
    std::mutex mtx; //对用户名单加锁，防止读写错误
    std::map<int, std::string> list; //存放每个用户的套接字和名字
    std::map<int, bool> is_private_chat; // 用于记录该用户是否进入私聊模式
    std::map<int, bool> is_group_chat;// 用于记录该用户是否进入群聊模式
    int server_fd; //服务器套接字
    std::vector<int> client_fds; //存放每个客户端的套接字    
public:
    server();
    void init(); //初始化
    void login(int client_fd); //添加用户名单
    void deleteuser(int client_fd); //删除断连用户
    void chat(int client_fd);
    int Send(int client_fd, int target_fd, std::string mode, const char* message);
    Message Recv(int client_fd);
    int privateChat(int client_fd, int partner_fd); //私聊
    int groupChat(int client_fd); //群聊
    ~server();
};

#endif // LAB1_SERVER_H