#include "lab1_server.h"
using namespace std;

server::server() {

}

void server::init() {
    if ((this->server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        cout << "Socket failed." << endl;
        return;
    }
    //创建地址族
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT); //主机转换网络
    addr.sin_addr.s_addr = INADDR_ANY; //接收任何网络

    //绑定套接字和地址，返回-1就失败
    if (0 != bind(server_fd, (sockaddr*)&addr, sizeof(addr))) {
        cout << "Connect has been failed." << endl;
        return;
    }
    //监听客户端
    if (listen(server_fd, 12) == -1) {
        cout << "Listen failed." << endl;
        return;
    }
    cout << "The server has been listening." << endl;

    //通信
    while (1) { //让服务器一直处于监听状态
        char ip[32] = { 0 };
        sockaddr_in caddr;
        socklen_t len = sizeof(sockaddr_in);
        int client_fd = accept(this->server_fd, (sockaddr*)&caddr, &len); //接受到的客户端套接字
        if (client_fd == -1) {
            cout << "客户端连接断开" << endl;
            continue;
        }
        inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip));
        cout << "来自ip:" << ip << " 端口号：" << ntohs(caddr.sin_port) << "连接成功" << endl; //客户端产生的端口号是随机的
        thread t(&server::chat, this, client_fd);
        t.detach();
    }
}

void server::login(int client_fd) {
    char buff[BUFFER_SIZE] = { 0 };
    // 向客户端发送系统消息：请输入用户名
    strcpy(buff, "请输入用户名:");
    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
        return;
    }
    string username;
    while(1){
        while(this->Send(0,client_fd,SYSTEM_CODE,LOGIN_CODE) == -1); // 向客户端发送输入用户名的指令
        Message m = this->Recv(client_fd);
        if(m.mode_code == LOGIN){ // 模式：“LOGIN|<Time>|<Username>|SERVER_STRING|NONE_STRING”
            username = m.source_name;
            break;
        }
        strcpy(buff, "输入错误，请重新输入用户名:");
        if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
            return;
        }
        
    }
    
    {
        unique_lock<mutex> lock(mtx);
        client_fds.push_back(client_fd);
        // cout<<"client_fd: "<< client_fd<<" username:" << username << endl;
        // list.insert(make_pair(client_fd, username));
        list[client_fd] = username;
        is_private_chat[client_fd] = false;
        is_group_chat[client_fd] = false;
    }

    // cout << "username: " <<username<<endl;
    // cout << "test:" << (list.empty()) << endl << "-----------" << endl;
    // for (auto it = list.begin(); it != list.end(); ++it) {
    //     int key = it->first;  // 获取键
    //     std::string value = it->second;  // 获取值
    //     cout<<"first: " << key << " second: " << value << endl;
    // }
    // cout << "--------------" << endl;

    strcpy(buff, "登陆成功！");
    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
        return;
    }
    while(this->Send(0,client_fd,SYSTEM_CODE,LOGIN_SECCESS_CODE) == -1); // 向客户端发送登陆成功的指令
    cout << username << " has logged in." << endl;
}

void server::deleteuser(int client_fd) {
    {
        unique_lock<mutex> lock(mtx);
        client_fds.erase(remove(client_fds.begin(), client_fds.end(), client_fd), client_fds.end());
        list.erase(client_fd);
        is_private_chat.erase(client_fd);
        is_group_chat.erase(client_fd);
    }
    close(client_fd);
}

int server::Send(int client_fd, int target_fd, string mode, const char* message){
    Message m;
    // 总模式: “  <ModeName>   |<Time>|<Username>   |<TargetUsername>|<Message/Mode>”，不需要的时候直接空着
    // 模式： “ SYSTEM_MESSAGE |<Time>|SERVER_STRING|<TargetUsername>|<Massage>     ”
    // 模式： “ PRIVATE_MESSAGE|<Time>|<Username>   |<TargetUsername>|<Message>     ”
    // 模式： “ GROUP_MESSAGE  |<Time>|<Username>   |SERVER_STRING   |<Message>     ”
    // 模式： “ QUIT           |<Time>|SERVER_STRING|<TargetUsername>|NONE_STRING   ”
    // 模式： “ SYSTEM_MODE    |<Time>|SERVER_STRING|<TargetUsername>|<Message>     ”
    m.mode_code = mode;
    m.time = getNowTime();
    m.source_name = (mode == SYSTEM_MESSAGE || mode == QUIT) ? SERVER_STRING : list[client_fd];
    // 发出LOGIN_CODE时还没有获取用户名
    m.target_name = (mode == GROUP_MESSAGE) ? SERVER_STRING : ((mode == SYSTEM_CODE && message == string(LOGIN_CODE)) ? NONE_STRING : list[target_fd]); 
    m.message = (mode == QUIT) ? NONE_STRING : string(message);
    string temp = Encode(m);
    //发送消息
    if(send(target_fd, temp.c_str(), temp.length() + 1, 0) < 0) {
		cout << "Server error : message send failed." << endl;
        cout << "Message content is: " << temp << endl;
        return -1;
	}
    // cout << "Server send: " << temp << endl; 
    usleep(100*1000);
    return 0;
}

Message server::Recv(int client_fd){
    char buff[BUFFER_SIZE] = { 0 };
    Message m;
    int bytesReceived = recv(client_fd, buff, sizeof(buff), 0);
    if(bytesReceived > 0){
        string protocolMessage(buff, bytesReceived);
        m = Decode(protocolMessage);
        // cout << "Server receive: " << protocolMessage << endl;
    } else if (bytesReceived == 0) { // 发送系统消息
        m.mode_code = SYSTEM_MESSAGE;
        m.mode_code = getNowTime();
        m.source_name = SERVER_STRING;
        m.target_name = list[client_fd];
        m.message = "Server error: Client disconnected.\n"; // 客户端关闭了连接
        cout << "Server error: Client disconnected." << endl;
    } else { // 发送系统消息
        m.mode_code = SYSTEM_MESSAGE;
        m.mode_code = getNowTime();
        m.source_name = SERVER_STRING;
        m.target_name = list[client_fd];
        m.message = "Server error: Receive failed.\n";
        cout << "Server error: Receive failed." << endl;
    }
    return m;
}

void server::chat(int client_fd) {
    char buff[1024] = { 0 };
    
    login(client_fd); // 登陆，获取用户名
    
    // 选择模式
    strcpy(buff, "请选择聊天模式：");
    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
        return;
    }
    strcpy(buff, "输入0--------------------进入私聊模式");
    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
        return;
    }
    strcpy(buff, "输入1--------------------进入群聊模式");
    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
        return;
    }

    string select;
    while(1){
        while(this->Send(0,client_fd,SYSTEM_CODE,SELECT_MODE_CODE) == -1);// 向客户端发送选择模式的指令
        Message m = this->Recv(client_fd);
        if(m.mode_code == CHOOSE_MODE){ // 模式：“LOGIN|<Time>|<Username>|SERVER_STRING|NONE_STRING”
            select = m.message;
            // select.erase(select.find_last_not_of('\0') + 1);
            if(select == "0" || select == "1"){
                break;
            }
        }
        strcpy(buff, "模式输入错误，请重新输入模式:");
        if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
            return;
        }
    }

    while(this->Send(0,client_fd,SYSTEM_CODE,SELECT_MODE_SECCESS_CODE) == -1);// 向客户端发送模式选择成功的指令

    if(select == "0"){ // 私聊模式
        is_private_chat[client_fd] = true;
        strcpy(buff, "您已进入私聊模式,输入QUIT即可退出");
        if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
            return;
        }
        string partner_name;
        while(1){ 
            vector<string> names; // 在线人员名单
            for (auto& it : list) {
                if (is_private_chat[it.first] && it.first != client_fd) {
                    names.push_back(it.second);
                }
            }
            if(names.size() == 0){
                strcpy(buff, "当前无人在线,请耐心等待");
                if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
                    return;
                }
                usleep(1000*1000); // 每过1秒刷新一次
                continue;
            }
            else{
                string temp("在线:"); // 发送当前在线人员名单
                for(auto name : names){
                    temp = temp + " " + name;
                }
                if(this->Send(0,client_fd,SYSTEM_MESSAGE,temp.c_str()) == -1){
                    return;
                }
                strcpy(buff, "请输入你私聊的对象:");
                if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
                    return;
                }
                while(this->Send(0,client_fd,SYSTEM_CODE,CHOOSE_PARTNER_CODE) == -1);// 向客户端发送选择私聊对象S的指令
                Message partner = this->Recv(client_fd);
                if(partner.mode_code == CHOOSE_PARTNER){ // 模式：“CHOOSE_PARTNER|<Time>|<Username>|<TargetUsername>|NONE_STRING”
                    bool judge = false; // 判断输入的对象是否上线
                    for(auto name : names){
                        if(name == partner.target_name){ // 私聊对象匹配成功
                            judge = true;
                            partner_name = partner.target_name;
                            break; // 此时跳出循环
                        }
                    }
                    if(!judge){ // 如果输入的对象没有匹配成功，认为其没有上线，需要重新输入名称
                        strcpy(buff, "你输入的私聊对象没有上线，请重新输入你私聊的对象");
                        if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
                            return;
                        }
                        continue;
                    }
                    else{ // 输入对象匹配成功，跳出循环
                        break;
                    }
                }
                else{
                    strcpy(buff, "客户端发送消息类型错误");
                    if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
                        return;
                    }
                    continue;
                }
            }
        }

        while(this->Send(0,client_fd,SYSTEM_CODE,CHOOSE_PARTNER_SECCESE_CODE) == -1);// 向客户端发送私聊对象选择成功的指令

        int partner_fd;
        for (auto& it : list) { // 遍历找到私聊对象的套接字
            if (it.second == partner_name) {
               partner_fd = it.first;
            }
        }

        int status = this->privateChat(client_fd, partner_fd); // 建立其从client到partner的连接
        if(status == 0){  // 认为用户正常退出
            cout << list[client_fd] << "已下线" << endl;
            this->deleteuser(client_fd);
            return;
        }
        else if(status == -1){
            // 认为对方已经下线
            strcpy(buff,"您的聊天对象已经下线");
            if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
                return;
            }
            if(this->Send(0,partner_fd,QUIT,NONE_STRING) == -1){ // 发送退出消息
                return;
            }
            cout << list[client_fd] << "已下线" << endl;
            this->deleteuser(client_fd);
            return;
        }
        else{
            cout << "Server error: private chat." << endl;
            return;
        }

    } 
    else if(select == "1"){ // 群聊模式
        is_group_chat[client_fd] = true;
        strcpy(buff, "您已进入群聊模式,输入QUIT即可退出");
        if(this->Send(0,client_fd,SYSTEM_MESSAGE,buff) == -1){
            return;
        }
        int status = this->groupChat(client_fd);
        if(status == 0){ // 认为用户正常退出
            cout << list[client_fd] << "已下线" << endl;
            this->deleteuser(client_fd);
            deleteuser(client_fd);
            return;
        }
        else{
            cout << "Server error: group chat." << endl;
            return;
        }
    }
    else{
        cout << "Mode code error." << endl;
        return;
    }
}

int server::privateChat(int client_fd, int partner_fd) {
    cout<<"PrivateChat: " << list[client_fd] << " to " << list[partner_fd] << " begin." << endl; 
    while(1){
        Message m = this->Recv(client_fd);
        // 模式：“PRIVATE_MESSAGE|<Time>|<Username>|<TargetUsername>|<Message>”
        if(m.mode_code == QUIT){ // 模式： “ QUIT|<Time>|<Username>|SERVER_STRING|NONE_STRING”
            // 退出程序
            if(this->Send(0, client_fd, QUIT, NONE_STRING) == -1){ // 向请求对出的客户端发送退出指令
                return -2;
            }
            this->Send(0, partner_fd, SYSTEM_CODE, PARTNER_OFFLINE_CODE); // 告知对方客户端该客户端已经下线
            return 0;
        }
        else if(m.mode_code != PRIVATE_MESSAGE){
            // 这里可以处理客户端传来的非私聊消息
            cout<<"Server received un-private_message: "  << m.message << endl;
            continue;
        }
        {
            unique_lock<mutex> lock(mtx);
            if(this->Send(client_fd, partner_fd, PRIVATE_MESSAGE, m.message.c_str()) == -1){
                return -1;
            }
        }
    }
    return -1;
}

int server::groupChat(int client_fd) {
    while (1) {
        Message m = this->Recv(client_fd);
        // 模式：“GROUP_MESSAGE:<Time>|<Username>|SERVER_STRING|<Message>”
        if(m.mode_code == QUIT){ // 模式： “ QUIT|<Time>|<Username>|SERVER_STRING|NONE_STRING”
            // 退出程序
            if(this->Send(0, client_fd, QUIT, NONE_STRING) == -1){
                return -1;
            }
            return 0;
        }
        else if(m.mode_code != GROUP_MESSAGE){
            // 这里可以处理客户端传来的非群聊消息
            continue;
        }
        // 广播
        auto key = list.find(client_fd); //先看map中是否存在该key值
        if (key != list.end()) {
            string str = "[" + key->second + "]:" + m.message;
            {
                unique_lock<mutex> lock(mtx);
                for (auto& fd : client_fds) {
                    if(is_group_chat[fd]){
                        this->Send(client_fd,fd,GROUP_MESSAGE,str.c_str());
                    }
                }
            }
        }
    }
    return -1;
}

server::~server() {
    close(server_fd);
}