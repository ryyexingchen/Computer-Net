#include"lab1_client.h"
using namespace std;


client::client(): client_fd(0), selected_mode("2"), login_status(false), select_mode(false), choose_partner(false), quit_status(false) {}

int client::init(){
    // 创建socket
    if ((this->client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Could not create socket." << endl;
        return -1;
    }

    // 设置服务器地址
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if (connect(this->client_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        cout << "Connect has been failed." << endl;
        return -1;
    }

    cout << "Connected to server successfully." << endl;
    return 0;
}

int client::login(){
    cin >> this->username;
    if(this->Send(LOGIN,NULL,SERVER_STRING,NONE_STRING) == -1){
        cout << "Message send failed." << endl;
        return -1;
    }
    return 0;
}
int client::selectMode(){
    string select;
    cin >> select;
    if(this->Send(CHOOSE_MODE,NULL,SERVER_STRING, select) == -1){
        return -1;
    }
    this->selected_mode = select;
    return 0;
}

int client::choosePartner(){
    string temp;
    cin >> temp;
    if(this->Send(CHOOSE_PARTNER,NULL,temp,NONE_STRING) == -1){
        return -1;
    }
    this->target_name = temp;
    return 0;
}

int client::chat(){
    thread t(&client::serverRecv,this);
	t.detach();
    while(!login_status){ usleep(100*1000); } // 登录成功后才可以继续

    while(!select_mode){ usleep(100*1000); } //选择模式后才能继续
	
	if (this->selected_mode == "0") { //私聊模式
		// usleep(100*1000);
		while(!choose_partner){ usleep(1000); } // 选择私聊对象后才能继续
		cout << "Chat begin." << endl;
        usleep(100*1000);
		while (!quit_status) {
			char buff[BUFFER_SIZE] = { 0 };
			cin.getline(buff, BUFFER_SIZE);
            if(isBuffBlank(string(buff))){
                cout << "你输入的字符串为空，请重新输入" << endl;
                memset(buff,0,BUFFER_SIZE);
                continue;
            }
            if(!isBuffLegal(string(buff))){
                cout << "你输入的字符串包含'|'，请重新输入" << endl;
                memset(buff,0,BUFFER_SIZE);
                continue;
            }
            if(string(buff) == string(QUIT_CODE)){ // 模式：”QUIT|<Time>|<Username>|<Server>|NONE_STRING“
                if(this->Send(QUIT,NULL,SERVER_STRING,NONE_STRING) == -1){
                    return -1;
                }
                break;
            }
            else{
                if(this->Send(PRIVATE_MESSAGE,buff,this->target_name,NONE_STRING) == -1){
                    return -1;
                }
            }
		}
	}
	else if(this->selected_mode == "1"){ //群聊模式
        cout<<"欢迎进入群聊聊天室"<<endl;
        usleep(100*1000);
		while (!quit_status) {
			char buff[BUFFER_SIZE] = { 0 };
			cin.getline(buff, BUFFER_SIZE);
            if(isBuffBlank(string(buff))){
                cout << "你输入的字符串为空，请重新输入" << endl;
                memset(buff,0,BUFFER_SIZE);
                continue;
            }
            if(!isBuffLegal(string(buff))){
                cout << "你输入的字符串包含'|'，请重新输入" << endl;
                memset(buff,0,BUFFER_SIZE);
                continue;
            }
            if(string(buff) == string(QUIT_CODE)){ // 模式：”QUIT|<Time>|<Username>|<Server>|NONE_STRING“
                if(this->Send(QUIT,NULL,SERVER_STRING,NONE_STRING) == -1){
                    return -1;
                }
                break;
            }
            else{
                if(this->Send(GROUP_MESSAGE,buff,SERVER_STRING,NONE_STRING) == -1){
                    return -1;
                }
            }
		}
	}
    cout<<"Client quit."<<endl;
    return 0;
}

int client::Send(string mode, char* message, string target_username, string chosen_mode=NONE_STRING){
    Message m;
    // 总模式:“   <ModeName>   |<Time>|<Username>|<TargetUsername>|<Message/Mode>”
    // 模式： “ LOGIN          |<Time>|<Username>|SERVER_STRING   |NONE_STRING   ”
    // 模式： “ CHOOSE_MODE    |<Time>|<Username>|SERVER_STRING   |<Mode>        ”
    // 模式： “ CHOOSE_PARTNER |<Time>|<Username>|<TargetUsername>|NONE_STRING   ”
    // 模式： “ PRIVATE_MESSAGE|<Time>|<Username>|<TargetUsername>|<Message>     ”
    // 模式： “ GROUP_MESSAGE  |<Time>|<Username>|SERVER_STRING   |<Message>     ”
    // 模式： “ QUIT           |<Time>|<Username>|SERVER_STRING   |NONE_STRING   ”
    m.mode_code = mode;
    m.time = getNowTime();
    m.source_name = this->username;
    m.target_name = (mode == CHOOSE_PARTNER || mode == PRIVATE_MESSAGE) ? target_username : SERVER_STRING;
    m.message = (mode == PRIVATE_MESSAGE || mode == GROUP_MESSAGE) ? string(message) : chosen_mode; // chosen_mode默认指为NONE_STRING

    string temp = Encode(m);
    
    if (send(this->client_fd, temp.c_str(), temp.length() + 1, 0) < 0){
            cout << "Client error : message send failed." << endl;
            return -1;
    }

    // cout << "Client send:" << temp << endl;
    usleep(100*1000);
    return 0;
}

Message client::Recv(){
    char buff[BUFFER_SIZE] = { 0 };
    Message message;
    int bytesReceived = recv(this->client_fd, buff, sizeof(buff), 0);
    if(bytesReceived > 0){
        string protocolMessage(buff, bytesReceived);
        message = Decode(protocolMessage);
        // cout << "Client receive: " << protocolMessage << endl;
    } else if (bytesReceived == 0) { // 发送系统消息
        message.mode_code = SYSTEM_MESSAGE;
        message.mode_code = getNowTime();
        message.source_name = this->username;
        message.target_name = SERVER_STRING;
        message.message = "Client error: Server disconnected.\n"; // 服务端关闭了连接
        cout << "Client error: Server disconnected." << endl;
    } else { // 发送系统消息
        message.mode_code = SYSTEM_MESSAGE;
        message.mode_code = getNowTime();
        message.source_name = this->username;
        message.target_name = SERVER_STRING;
        message.message = "Client error: Receive failed.\n";
        cout << "Client error: Receive failed." << endl;
    } 
    return message;
}

void client::serverRecv(){
    // 总模式: “  <ModeName>   |<Time>|<Username>   |<TargetUsername>|<Message/Mode>”，不需要的时候直接空着
    // 模式： “ SYSTEM_MESSAGE |<Time>|SERVER_STRING|<TargetUsername>|<Massage>     ”
    // 模式： “ PRIVATE_MESSAGE|<Time>|<Username>   |<TargetUsername>|<Message>     ”
    // 模式： “ GROUP_MESSAGE  |<Time>|<Username>   |SERVER_STRING   |<Message>     ”
    // 模式： “ QUIT           |<Time>|SERVER_STRING|<TargetUsername>|NONE_STRING   ”
    while (1) {
        Message m = this->Recv();
        // cout << m.message << " " << LOGIN_CODE << " " << (m.message == LOGIN_CODE) << " " << (m.message.compare(LOGIN_CODE) == 0)<< endl;
		{
			lock_guard<mutex> lock(this->mtx);
			if(m.mode_code == SYSTEM_MESSAGE){
                cout << "[" << m.time << "] 来自服务器的消息: " << m.message << endl;
                continue;
            }
            else if(m.mode_code == PRIVATE_MESSAGE){
                cout << "[" << m.time << "] 来自" << m.source_name << "的消息: " << m.message << endl;
                continue;
            }
            else if(m.mode_code == GROUP_MESSAGE){
                cout << "[" << m.time << "] 来自群聊的消息: " << m.message << endl;
                continue;
            }
            else if(m.mode_code == QUIT){
                this->quit_status = true;
                cout << "[" << m.time << "] 来自服务器的消息: 确认退出" << endl;
                return;
            }
            else if(m.mode_code == SYSTEM_CODE){
                if(m.message == LOGIN_CODE){
                    while(this->login() == -1) { usleep(50*1000); } ;
                }
                else if(m.message == LOGIN_SECCESS_CODE){
                    this->login_status = true;
                }
                else if(m.message == SELECT_MODE_CODE){
                    this->selectMode();
                }
                else if(m.message == SELECT_MODE_SECCESS_CODE){
                    this->select_mode = true;
                }
                else if(m.message == CHOOSE_PARTNER_CODE){
                    this->choosePartner();
                }
                else if(m.message == CHOOSE_PARTNER_SECCESE_CODE){
                    this->choose_partner = true;
                }
                else if(m.message == PARTNER_OFFLINE_CODE){
                    cout << "[" << m.time << "] 来自服务器的消息: 您的私聊对象已经退出聊天室" << endl;
                    while(this->Send(QUIT,NULL,SERVER_STRING,NONE_STRING) == -1) { usleep(100*1000); } // 向服务器申请退出
                }
                continue;
            }
            else{
                cout << "Client receive error!" << endl;
                return;
            }
		}
		usleep(1000);//给输入,打印留抢占时间
	}
}

client::~client(){
    close(this->client_fd); // 关闭套接字
}