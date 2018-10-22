#include "server.h"
/* =============================================
 * 基础处理函数
 * =============================================
 */
 
/* 函数功能：创建套接字并监听
 * 传入参数：
 * -port：端口号
 * 返回值：错误返回-1;正确则返回套接字描述符。
 */
int create_socket(int port)
{	
	int sockfd;
	int flag = 1;
	struct sockaddr_in addr;

	//创建套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//设置套接字地址与其状态
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) 
	{
		close(sockfd);
		printf("Error setsockopt(): %s(%d)\n", strerror(errno), errno);
		return -1; 
	}
	
	//绑定套接字地址
	if(bind(sockfd,(struct sockaddr *) &addr,sizeof(addr)) < 0)
	{
		close(sockfd);
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//开始监听socket
	if (listen(sockfd, 10) < 0) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	return sockfd;
	
}

/* 函数功能：处理套接字接受请求
 * 传入参数：
 * -listenfd：正在监听的套接字
 * 返回值：错误返回-1;正确则返回新的套接字描述符。
 */
int accept_socket(int listenfd)
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	sockfd = accept(listenfd, (struct sockaddr *) &addr, &len);
	
	if (sockfd < 0)
	{
		printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		return -1; 
	}
	return sockfd;	
}

/* 函数功能：接收客户端的命令并处理
 * 传入参数：
 * -sockfd：信息传输的套接字
 * -cmd：客户端命令
 * -param：客户端命令参数
 * 返回值：错误返回-1;正确则返回0。
 */
int receive_cmd(int sockfd,char *cmd,char *param)
{
	size_t num_bytes;
	char buffer[MAX_SIZE];
	memset(buffer, 0, MAX_SIZE);

	num_bytes = recv(sockfd, buffer, MAX_SIZE, 0);
	if (num_bytes < 0)
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	cmd = strtok(buffer, " ");
	param = strtok(NULL, " ");
}

/* 函数功能：发送数据
 * 传入参数：
 * -sockfd：数据传输的套接字
 * -buf：缓冲区指针
 * -bufsize：缓冲区大小
 * 返回值：错误返回-1;正确则返回发送数据的字节数。
 */
int send_data(int sockfd,char *buf,int bufsize)
{
	size_t num_bytes;

	num_bytes = send(sockfd, buf, bufsize, 0);
	if (num_bytes < 0)
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	return num_bytes;	
}

/* =============================================
 * 服务器相关函数
 * =============================================
 */

/* 函数功能：服务器运行的主要控制函数
 * 传入参数：
 * -argc：控制台参数数量
 * -argv：控制台参数
 * 返回值：0
 */
int server_work(int argc,char **argv)
{
	int listenfd,controlfd;
	int pid = 1;
	
	//处理控制台输入参数
	for(int i=1;i<argc;i++)
	{
		if(!strcmp(argv[i],"-port"))
		{
			PORT = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i],"-root"))
		{
			FILE_ROOT=argv[++i];
		}
		else
		{
			printf("usage: ./server -port [number] -root [string]\n");
			exit(0);
		}
	}
	
	// 创建监听套接字 
	if ((listenfd = create_socket(PORT)) < 0 )
	{
		printf("Error create socket!");
		exit(1);
	}
	
	//使用多进程接受多个客户端的请求
	while(1)
	{
		if((controlfd = accept_socket(listenfd))<0)
		{
			printf("Something wrong!");
			break;
		}
		
		if((pid=fork())<0)
		{
			printf("Fail to create subprocess!");
		}
		
		else if (!pid)
		{
			server_process(controlfd);
			close(controlfd);
			close(listenfd);
			exit(0);
		}
		
		close(controlfd);
	}
	
	close(listenfd);
	
	return 0;
}

/* 函数功能：服务器与每个客户端的交互进程
 * 传入参数：
 * -controlfd：控制进程套接字
 * 返回值：无
 */
void server_process(int controlfd)
{
	int datafd;//数据传输套接字
	char cmd[5];//客户端命令（verb）
	char param[MAX_SIZE];//客户端命令参数
	
	char prompt1[] = "220 FTP server ready.";
	
	//首先返回成功连接信号
	send_data(controlfd,prompt1,strlen(prompt1));
	
}

int main(int argc, char **argv) {

	//开启服务器活动
	server_work(argc,argv);
	
	return 0;
}

