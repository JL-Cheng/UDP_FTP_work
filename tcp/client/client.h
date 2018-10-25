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
#define COMMAND_NUM 16

char *CLIENT_INSTRUCTION[]=
{
	"USER",	"PASS", 
	"RETR", "STOR", 
	"QUIT", "SYST", 
	"TYPE", "PORT", 
	"PASV", "MKD", 
	"CWD", "PWD",
	"LIST", "RMD", 
	"RNFR", "RNTO" 
};


/* =============================================
 * 客户端相关函数
 * =============================================
 */

//客户端运行的主要控制函数
int client_work(int argc,char **argv);

//客户端PORT命令的控制函数
int client_port(char* param,char *ip,int *port);

//客户端PASV命令的控制函数
int client_pasv(int sockfd,char *buffer,char *ip,int *port);

//客户端RETR命令控制函数
int client_retr(int sockfd,char *buffer,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port);

//客户端STOR命令控制函数
int client_stor(int sockfd,char *buffer,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port);


/* =============================================
 * 基础处理函数
 * =============================================
 */
//创建套接字并监听
int create_socket(int port);

//处理套接字接受请求
int accept_socket(int listenfd);

//发送数据
int send_data(int sockfd,char *buf,int bufsize);

//从命令行中读取输入
int get_input(char* buffer, int size,char *cmd,char *param);

//得到ip地址和端口port
int get_ip_port(char *param,char *ip,int *port);

#endif
