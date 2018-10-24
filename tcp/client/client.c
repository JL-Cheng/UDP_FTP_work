#include"client.h"

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
	addr.sin_port = htons(port);
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

/* 函数功能：从命令行读取参数
 * 传入参数：
 * -buffer：缓冲区
 * -size：缓冲区大小
 * -cmd：命令
 * -param：命令参数
 * 返回值：输入正确返回0,否则返回-1.
 */
int get_input(char* buffer, int size,char *cmd,char *param)
{
	char *ch = 0;
	char *cmd_t = 0;
	char *param_t = 0;
	char buffer_t[size];
	int cmd_valid = 0;
	
	memset(buffer, 0, size);
	memset(buffer_t, 0, size);
	memset(cmd,0,5);
	memset(param,0,MAX_SIZE);

	if(fgets(buffer, size, stdin))
	{
		ch = strchr(buffer, '\n');
		if (ch) 
			*ch = '\0';
	}
	
	if(strlen(buffer)==0)
		return -1;	
	
	strcpy(buffer_t,buffer);
	//判断cmd的合法性
	cmd_t = strtok(buffer_t, " ");
	if(cmd_t)
	{
		for(int i=0;i<COMMAND_NUM;i++)
		{
			if(!(strcmp(cmd_t,CLIENT_INSTRUCTION[i])))
			{
				cmd_valid = 1;
				break;
			}
		}
	}
	if(!cmd_valid)
	{
		printf("Invalid command!\n");
		return -1;	
	}
	
	//判断param合法性
	param_t = strtok(NULL, " ");
	if(param_t && (strlen(param_t) >= MAX_SIZE))
	{
		printf("Syntax error in parameters!\n");
		return -1;
	}

	strcpy(cmd,cmd_t);
	if(param_t)
		strcpy(param,param_t);
		
	return 0;
}

/* 函数功能：得到IP地址和port端口号
 * 传入参数：
 * -param：带有ip和port的字符串
 * -ip：返回的ip地址
 * -port：返回的端口号
 * 返回值：错误返回-1;正确返回0。
 */
int get_ip_port(char *param,char *ip,int *port)
{
	//检测参数
	char *ch;
	
	*port = 0;
	memset(ip,0,20);
	
	ch = strtok(param, ",");
	if(!ch || strlen(ch)>3)
	{
		printf("Wrong port param.\n");
		return -1;
	}
	else
	{
		for(int i=0;i<strlen(ch);i++)
		{
			if(ch[i]<'0' || ch[i]>'9')
			{
				printf("Wrong port param.\n");
				return -1;	
			}
		}
	}
	
	strcpy(ip,ch);
	for(int i=0;i<3;i++)
	{
		ch = strtok(NULL, ",");
		if(!ch || strlen(ch)>3)
		{
			printf("Wrong port param.\n");
			return -1;
		}
		else
		{
			for(int i=0;i<strlen(ch);i++)
			{
				if(ch[i]<'0' || ch[i]>'9')
				{
					printf("Wrong port param.\n");
					return -1;	
				}
			}
		}
		strcat(ip,".");
		strcat(ip,ch);
	}
	
	for(int i=0;i<2;i++)
	{
		ch = strtok(NULL, ",");
		if(!ch)
		{
			printf("Wrong port param.\n");
			return -1;
		}
		else
		{
			for(int i=0;i<strlen(ch);i++)
			{
				if(ch[i]<'0' || ch[i]>'9')
				{
					printf("Wrong port param.\n");
					return -1;	
				}
			}
		}
		*port = *port *256 + atoi(ch);
	}
	
	if(*port < 20000 || *port > 65535)
	{
		printf("Port should range from 20000 to 65535.\n");
		return -1;
	}	
	
	return 0;
}


/* =============================================
 * 客户端相关函数
 * =============================================
 */

/* 函数功能：客户端运行的主要控制函数
 * 传入参数：
 * -argc：控制台参数数量
 * -argv：控制台参数
 * 返回值：正确返回0,否则返回1
 */
int client_work(int argc,char **argv)
{
	struct sockaddr_in addr;
	
	char buffer[MAX_SIZE];
	char cmd[5];
	char param[MAX_SIZE];
    int sockfd = -1;
    int datafd = -1;
	int len=0;
	
	char PORT_ip[20];//PORT指令数据传输ip地址
	int PORT_port = 0;//PORT指令数据传输端口
	char PASV_ip[20];//PASV指令数据传输ip地址
	int PASV_port = 0;//PASV指令数据传输端口
	int is_PORT = 0;//使用PORT命令主动传输
	int is_PASV = 0;//使用PASV命令被动传输

	//创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  
	//设置目标主机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(21);
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) 
	{
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  

	//连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return 1;
	}
	//接收连接成功消息
	if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	buffer[len]='\0';
	printf(">>>> %s", buffer);


	while (1) 
	{
		//获得输入
		printf("fcp> ");
		if(get_input(buffer,MAX_SIZE,cmd,param))
			continue;

		//判断命令
		if(!(strcmp(cmd,"PORT")))//"PORT"
		{
			if(client_port(param,PORT_ip,&PORT_port) < 0)
				continue;
			is_PORT = 1;
			is_PASV = 0;
		}
		
		else if(!(strcmp(cmd,"PASV")))//"PASV"
		{
			if(client_pasv(sockfd,buffer,PASV_ip,&PASV_port) < 0)
			{
				close(sockfd);
				return 1;
			}
			is_PORT = 0;
			is_PASV = 1;
			continue;
		}
		
		//发送命令
		if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
		{
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}

		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		
		buffer[len]='\0';
			
		printf(">>>> %s", buffer);

	} // 循环得到更多的用户输入
 
    close(sockfd);

	return 0;

}

/* 函数功能：客户端PORT命令的控制函数
 * 传入参数：
 * -param：ip地址和端口号的参数
 * -ip：传入的ip地址
 * -port：传入的端口
 * 返回值：参数均正确返回0,否则返回-1。
 */
int client_port(char* param,char *ip,int *port)
{
	if(get_ip_port(param,ip,port)<0)
		return -1;
	return 0;
	
	/*int listenfd = create_socket(port);	
	if(listenfd<0)
	{
		printf("Create listenfd error.\n");
		return -1;
	}
	int datafd = accept_socket(listenfd);
	if(datafd<0)
	{
		printf("Create datafd error.\n");
		return -1;
	}
	close(listenfd);
	
	return datafd;*/
	
}

/* 函数功能：客户端PASV命令的控制函数
 * 传入参数：
 * -sockfd：控制套接字
 * -buffer：传输消息
 * -ip：传入的ip地址
 * -port：传入的端口
 * 返回值：参数均正确返回0,否则返回-1。
 */
int client_pasv(int sockfd,char *buffer,char *ip,int *port)
{
	int len=0;
	char *ch;
	
	//发送命令
	if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}

	if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}
		
	buffer[len]='\0';			
	printf(">>>> %s", buffer);
	
	ch = strtok(buffer, " ");
	if(!strcmp(ch,"421"))
	{
		return -1;
	}
	else
	{
		ch = strtok(NULL, "(");
		ch = strtok(NULL,")");
		if(get_ip_port(ch,ip,port)<0)
			return -1;
	}
	
	return 0;
}


int main(int argc, char **argv) 
{

	client_work(argc,argv);	

	return 0;
}
