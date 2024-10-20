#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <functional>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>


#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define NONE_STRING "NONE"
#define SERVER_STRING "SERVER"
#define QUIT_CODE "QUIT"

// 信息格式码
#define SYSTEM_CODE "0"
#define SYSTEM_MESSAGE "1"
#define LOGIN "2"
#define CHOOSE_MODE "3"
#define CHOOSE_PARTNER "4"
#define PRIVATE_MESSAGE "5"
#define GROUP_MESSAGE "6"
#define QUIT "7"

// 服务器对客户端的指示码（在SYSTEM_CODE格式中使用）
#define LOGIN_CODE "0"
#define LOGIN_SECCESS_CODE "1"
#define SELECT_MODE_CODE "2"
#define SELECT_MODE_SECCESS_CODE "3"
#define CHOOSE_PARTNER_CODE "4"
#define CHOOSE_PARTNER_SECCESE_CODE "5"
#define PARTNER_OFFLINE_CODE "6"
