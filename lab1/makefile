# 定义编译器
CXX = g++

# 定义目录路径
OBJ_DIR = ./obj
HEAD_DIR = ./include/
SOURCE_DIR = ./source/
MAIN_DIR = ./

# 定义编译选项
CXXFLAGS = -Wall -std=c++11 -I$(HEAD_DIR)

# 定义链接选项
LDFLAGS = -lpthread

# 定义服务器目标文件
SERVER_TARGET = server
SERVER_SOURCES = $(MAIN_DIR)lab1_main_server.cpp $(SOURCE_DIR)lab1_server.cpp $(SOURCE_DIR)lab1_message_code.cpp
SERVER_OBJECTS = $(OBJ_DIR)/lab1_main_server.o $(OBJ_DIR)/lab1_server.o $(OBJ_DIR)/lab1_message_code.o

# 定义客户端目标文件
CLIENT_TARGET = client
CLIENT_SOURCES = $(MAIN_DIR)lab1_main_client.cpp $(SOURCE_DIR)lab1_client.cpp $(SOURCE_DIR)lab1_message_code.cpp
CLIENT_OBJECTS = $(OBJ_DIR)/lab1_main_client.o $(OBJ_DIR)/lab1_client.o $(OBJ_DIR)/lab1_message_code.o

# 默认目标
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# 构建服务器目标
$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $^ $(LDFLAGS)  -o $@

# 构建客户端目标
$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $^ $(LDFLAGS)  -o $@

# 编译源文件为目标文件
$(OBJ_DIR)/%.o: $(MAIN_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SOURCE_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建对象文件目录
$(shell mkdir -p $(OBJ_DIR))

# 清理编译生成的文件
clean: clean-obj
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)

# 清理对象文件
clean-obj:
	rm -rf $(OBJ_DIR)/*.o

# 伪目标
.PHONY: all clean clean-obj