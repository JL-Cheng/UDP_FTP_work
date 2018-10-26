#ifndef SERVER_H
#define SERVER_H

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
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

/* =============================================
 * 一些宏定义或全局数据
 * =============================================
 */
#define DEFAULT_PORT 21
#define DEFAULT_ROOT "/tmp"
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
int PORT = DEFAULT_PORT;
char FILE_ROOT[200];

/* =============================================
 * 服务器相关函数
 * =============================================
 */

//服务器运行的主要控制函数
int server_work(int argc,char **argv);

//服务器与每个客户端的交互进程
void server_process(int controlfd);

//服务器控制用户登录
void server_login(int controlfd);

//服务器TYPE命令控制函数
void server_type(int controlfd,char *param);

//服务器MKD命令控制函数
void server_mkd(int controlfd,char *param);

//服务器CWD命令控制函数
void server_cwd(int controlfd,char *param);

//服务器PORT命令的控制函数
int server_port(int controlfd,char *param,char *ip,int *port);

//服务器PASV命令的控制函数
int server_pasv(int controlfd);

//服务器RETR命令控制函数
int server_retr(int controlfd,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd);

//服务器STOR命令控制函数
int server_stor(int controlfd,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd);



/* =============================================
 * 基础处理函数
 * =============================================
 */

//创建套接字并监听
int create_socket(int port);

//处理套接字接受请求
int accept_socket(int listenfd);

//接收客户端的命令并处理
int receive_cmd(int sockfd,char *cmd,char *param);

//发送数据
int send_data(int sockfd,char *buf,int bufsize);

//得到ip地址和端口port
int get_ip_port(char *param,char *ip,int *port);


#endif
