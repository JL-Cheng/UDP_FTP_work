#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>	

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>

/* =============================================
 * 一些宏定义或全局数据
 * =============================================
 */
#define MAX_SIZE 1024


/* =============================================
 * 客户端相关函数
 * =============================================
 */

//客户端运行的主要控制函数
int client_work(int argc,char **argv);



/* =============================================
 * 基础处理函数
 * =============================================
 */

//从命令行中读取输入
int get_input(char* buffer, int size);



#endif
