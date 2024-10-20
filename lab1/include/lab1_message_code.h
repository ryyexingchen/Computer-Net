#ifndef LAB1_MESSAGE_CODE_H
#define LAB1_MESSAGE_CODE_H

#include"lab1.h"

struct Message{
    std::string mode_code;
    std::string time;
    std::string source_name;
    std::string target_name;
    std::string message;
};

std::string Encode(Message message);
Message Decode(std::string message);
std::string getNowTime();
bool isBuffBlank(const std::string& str);
bool isBuffLegal(const std::string& str);

#endif // LAB1_SERVER_HSS