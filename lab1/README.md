# 聊天小程序

## 功能

### 可以实现两人私聊或者是多人群聊

## 配置要求

### linux系统，有g++环境和pthread库

## 使用方式
### 1、将文件copy到本地linux环境下
### 2、在根目录下输入 `make` 指令，可以编译出 `server` 和 `client` 两个可执行文件
### 3、输入指令 `./server` 运行服务器后，再输入指令 `./client` 运行客户端。可以同时运行多个客户端。

## 备注
### 1、在每次编译之前一定要输入 `make clean-obj` 指令清理编译生成的对象文件。推荐在每次make完之后输入指令 `make clean-obj` 进行清理。(输入 `make clean` 指令会把可执行文件也清理掉，请注意。)
### 2、存在已知bug:客户端有一定概率会在输入模式后卡死，推测原因为网络bug（包括丢包一类的问题）。修复方法: `ctrl+C` 杀死进程之后重新输入 `./client` 开启一个新的客户端（如果此时服务器陷入无限循环的话也需要对服务器进行同样操作）。