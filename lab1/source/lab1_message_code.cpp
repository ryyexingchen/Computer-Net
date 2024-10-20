#include "lab1_message_code.h"
using namespace std;

string Encode(Message message){
    string temp;
    // 总模式：“<ModeName>|<Time>|<Username>|<TargetUsername>|<Message/Mode>”
    if(message.mode_code == SYSTEM_MESSAGE){ // 模式：“SYSTEM_MESSAGE|<Time>|SERVER_STRING|<TargetUsername>|<Message>"
        temp = SYSTEM_MESSAGE + string("|") + message.time + string("|") + SERVER_STRING + string("|") + message.target_name + string("|") + message.message;
    }
    else if(message.mode_code == LOGIN){ // 模式：“LOGIN|<Time>|<Username>|SERVER_STRING|NONE_STRING”
        temp = LOGIN + string("|") + message.time + string("|") + message.source_name + string("|") + SERVER_STRING + string("|") + NONE_STRING;
    }
    else if(message.mode_code == CHOOSE_MODE){ // 模式：“CHOOSE_MODE|<Time>|<Username>|SERVER_STRING|<Mode>”
        temp = CHOOSE_MODE + string("|") + message.time + string("|") + message.source_name + string("|") + SERVER_STRING + string("|") + message.message;
    }
    else if(message.mode_code == CHOOSE_PARTNER){ // 模式：“CHOOSE_PARTNER|<Time>|<Username>|<TargetUsername>|NONE_STRING”
        temp = CHOOSE_PARTNER + string("|") + message.time + string("|") + message.source_name + string("|") + message.target_name + string("|") + NONE_STRING;
    }
    else if(message.mode_code == PRIVATE_MESSAGE){ // 模式：“PRIVATE_MESSAGE|<Time>|<Username>|<TargetUsername>|<Message>”
        temp = PRIVATE_MESSAGE + string("|") + message.time + string("|") + message.source_name + string("|") + message.target_name + string("|") + message.message;
    }
    else if(message.mode_code == GROUP_MESSAGE){ // 模式：“GROUP_MESSAGE|<Time>|<Username>|SERVER_STRING|<Message>”
        temp = GROUP_MESSAGE + string("|") + message.time + string("|") + message.source_name + string("|") + SERVER_STRING + string("|") + message.message;
    }
    else if(message.mode_code == QUIT){ // 模式：”QUIT|<Time>|<Username/Server>|<Server/Username>|NONE_STRING“
        temp = QUIT + string("|") + message.time + string("|") + message.source_name + string("|") + message.target_name + string("|") + NONE_STRING;
    }
    else if(message.mode_code == SYSTEM_CODE){ // 模式：”SYSTEM_CODE|<Time>|<Username>|<TargetUsername>|<Message>“
        temp = SYSTEM_CODE + string("|") + message.time + string("|") + message.source_name + string("|") + message.target_name + string("|") + message.message;
    }
    else{
        temp = SYSTEM_MESSAGE + string("|") + message.time + string("|") + SERVER_STRING + string("|") + message.target_name + string("|") + "Encode error: cannot find the mode pattern.\n";
    }
    return temp;
}

Message Decode(string message){
    // 总模式：“<ModeName>|<Time>|<Username>|<TargetUsername>|<Message/Mode>”
    size_t posMessageType = message.find(string("|"));
    size_t posTimestamp = message.find(string("|"), posMessageType + 1);
    size_t posSenderId = message.find(string("|"), posTimestamp + 1);
    size_t posReceiverId = message.find(string("|"), posSenderId + 1);
    size_t posMessageContent = message.rfind(string("|"));

    string messageType = message.substr(0, posMessageType); // 消息类型
    string time = message.substr(posMessageType + 1, posTimestamp - posMessageType - 1); // 时间
    string senderId = message.substr(posTimestamp + 1, posSenderId - posTimestamp - 1); // 消息来源
    string receiverId = message.substr(posSenderId + 1, posReceiverId - posSenderId - 1); // 消息对象
    string messageContent = message.substr(posMessageContent + 1); //消息
    messageContent.erase(messageContent.find_last_not_of('\0') + 1);

    Message temp;
    temp.mode_code = messageType;
    temp.time = time;
    temp.source_name = senderId;
    temp.target_name = receiverId;
    temp.message = messageContent;

    return temp;
}

string getNowTime(){
    auto now = chrono::system_clock::now();
    auto now_c = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&now_c), "%Y-%m-%d %X"); // 使用本地时间格式
    string time = ss.str();
    return time;
}

bool isBuffBlank(const string& str) {
    for (char c : str) {
        if (!isspace(c)) {
            return false; // 如果发现非空白字符，返回 false
        }
    }
    return true; // 字符串中只有空白字符或为空
}
bool isBuffLegal(const string& str) {
    for (char c : str) {
        if (c == '|') {
            return false; // 如果发现'|'，返回 false
        }
    }
    return true; // 字符串合法
}