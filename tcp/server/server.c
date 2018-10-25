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

/* 函数功能：接收客户端的命令并处理
 * 传入参数：
 * -sockfd：信息传输的套接字
 * -cmd：客户端命令
 * -param：客户端命令参数
 * 返回值：错误返回-1;正确则返回0。
 */
int receive_cmd(int sockfd,char * cmd,char *param)
{
	size_t num_bytes;
	char buffer[MAX_SIZE];
	char *cmd_t = 0;
	char *param_t = 0;
	int cmd_valid = 0;

	char prompt1[] = "500 Syntax error, command unrecognized.\n\r";
	char prompt2[] = "501 Syntax error in parameters.\n\r";

	memset(buffer, 0, MAX_SIZE);
	memset(cmd,0,5);
	memset(param,0,MAX_SIZE);

	num_bytes = recv(sockfd, buffer, MAX_SIZE, 0);
	if (num_bytes < 0)
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//判断cmd的合法性
	cmd_t = strtok(buffer, " ");
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
		send_data(sockfd,prompt1,strlen(prompt1));
		return -1;
		
	}
	
	//判断param合法性
	param_t = strtok(NULL, " ");
	if(param_t && (strlen(param_t) >= MAX_SIZE))
	{
		printf("Syntax error in parameters!\n");
		send_data(sockfd,prompt2,strlen(prompt2));
		return -1;
	}

	strcpy(cmd,cmd_t);
	if(param_t)
		strcpy(param,param_t);
		
	return 0;
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
			close(listenfd);
			server_process(controlfd);
			close(controlfd);
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
	int PASV_listenfd = -1;//PASV中监听传输套接字
	char PORT_ip[20];//PORT指令数据传输ip地址
	int PORT_port = 0;//PORT指令数据传输端口
	int is_PORT = 0;//使用PORT命令主动传输
	int is_PASV = 0;//使用PASV命令被动传输
	
	int flag;//状态参数
	char cmd[5];//客户端命令（verb）
	char param[MAX_SIZE];//客户端命令参数
	
	char prompt1[] = "220 Anonymous FTP server ready.\n\r";
	char prompt2[] = "502 Please use valid command.\n\r";

	//首先返回成功连接信号
	printf("success_connect\n");
	send_data(controlfd,prompt1,strlen(prompt1));
	
	//进行用户登录验证
	printf("user_login\n");
	//server_login(controlfd);
	
	//开启循环监听用户消息
	printf("start_work\n");
	while(1)
	{
		if(receive_cmd(controlfd,cmd,param))
			continue;
		//若是PORT指令
		if(!(strcmp(cmd,"PORT")))
		{
			flag = server_port(controlfd,param,PORT_ip,&PORT_port);
			if(flag < 0)//失败
			{
				memset(PORT_ip, 0, 20);
				PORT_port = 0;
				is_PORT=0;
			}
			else
			{
				is_PORT=1;
				is_PASV=0;
				if(PASV_listenfd > 0)
					close(PASV_listenfd);
				PASV_listenfd=-1;
			}
			continue;
		}
		//若是PASV指令
		else if(!(strcmp(cmd,"PASV")))
		{
			flag = server_pasv(controlfd);
			if(flag < 0)//失败
			{
				return;
			}
			else
			{
				is_PORT=0;
				is_PASV=1;
				if(PASV_listenfd > 0)
					close(PASV_listenfd);
				PASV_listenfd = flag;
				memset(PORT_ip, 0, 20);
				PORT_port = 0;
			}
			continue;
		}
		//若是RETR指令
		else if(!(strcmp(cmd,"RETR")))
		{
			if(server_retr(controlfd,param,is_PORT,PORT_ip,PORT_port,is_PASV,PASV_listenfd))
			{
				printf("Send file wrong.\n");
			}
			if(is_PORT)
			{
				is_PORT=0;
				memset(PORT_ip, 0, 20);
				PORT_port = 0;				
			}
			else if(is_PASV)
			{	
				is_PASV=0;
				PASV_listenfd=-1;
			}
			continue;
		}
		//若是其他命令
		else
		{
			send_data(controlfd,prompt2,strlen(prompt2));
			continue;
		}
	}

}

/* 函数功能：用于用户最初登录
 * 传入参数：
 * -controlfd：控制进程套接字
 * 返回值：无
 */
void server_login(int controlfd)
{
	char cmd[5];//客户端命令（verb）
	char param[MAX_SIZE];//客户端命令参数
	int user_ok = 0;//用户名是否正确
	
	char prompt1[] = "530 Please use 'USER' command to login first.\n\r";
	char prompt2[] = "332 Please enter correct username.\n\r";
	char prompt3[] = "331 User name is ok, send your email address as password.\n\r";
	char prompt4[] = "230 Guest login ok, now you can do what you want.\n\r";
	
	while(1)
	{
		if(receive_cmd(controlfd,cmd,param))
			continue;
		//首先必须是USER指令
		if(!(strcmp(cmd,"USER")) && !user_ok)
		{
			if(!(strcmp(param,"anonymous")))
			{
				send_data(controlfd,prompt3,strlen(prompt3));
				user_ok = 1;
			}
			else
			{
				send_data(controlfd,prompt2,strlen(prompt2));
			}
		}
		//其次输入邮箱，用户登录成功
		else if(!(strcmp(cmd,"PASS")) && user_ok)
		{
			send_data(controlfd,prompt4,strlen(prompt4));
			break;
		}
		else if(user_ok)
		{
			send_data(controlfd,prompt3,strlen(prompt3));
		}
		else if(!user_ok)
		{
			send_data(controlfd,prompt1,strlen(prompt1));
		}
	}	
	
	return;
}


/* 函数功能：服务器PORT命令的控制函数
 * 传入参数：
 * -controlfd：控制套接字
 * -param：ip地址和端口号的参数
 * -ip：传入的ip地址
 * -port：传入的端口
 * 返回值：参数均正确返回0,否则返回-1。
 */
int server_port(int controlfd,char *param,char *ip,int *port)
{

	char prompt1[] = "501 Syntax error in parameters or arguments.\n\r";
	char prompt2[] = "200 PORT command is ok.\n\r";
	
	if(get_ip_port(param,ip,port)<0)
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	
	send_data(controlfd,prompt2,strlen(prompt2));
	printf("PORT:\nip:%s\nport:%d\n",ip,*port);
	return 0;
	
}

/* 函数功能：服务器PASV命令的控制函数
 * 传入参数：
 * -controlfd：控制套接字
 * 返回值：正确返回listenfd,否则返回-1。
 */
int server_pasv(int controlfd)
{
	char prompt1[50] = "227 Entering Passive Mode ";
	char prompt2[] = "421 Service not available, closing control connection.\n\r";
	
	int port = 0;
	char ip[20];
	char p[5];
	char *ch;
	
	//随机生成port
	srand((unsigned)time(NULL));
	port = 	rand() % 45535 + 20000;
	
	printf("PASV:\nport:%d\n",port);
	//开始监听
	int listenfd = create_socket(port);	
	if(listenfd<0)
	{
		printf("Create listenfd error.\n");
		send_data(controlfd,prompt2,strlen(prompt2));
		return -1;
	}
	
	struct sockaddr_in addr;
	socklen_t len = sizeof addr;
	getsockname(controlfd, (struct sockaddr*)&addr, &len); // 获得服务器ip
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
	
	printf("ip:%s\n",ip);
		
	strcat(prompt1,"(");
	ch = strtok(ip, ".");
	strcat(prompt1,ch);
	for(int i=0;i<3;i++)
	{
		ch = strtok(NULL, ".");
		strcat(prompt1,",");
		strcat(prompt1,ch);
	}
	
	memset(p,0,5);
	sprintf(p, "%d", port / 256); 
	strcat(prompt1,",");
	strcat(prompt1,p);
	memset(p,0,5);
	sprintf(p, "%d", port % 256); 
	strcat(prompt1,",");
	strcat(prompt1,p);
	strcat(prompt1,")\n\r");
	
	send_data(controlfd,prompt1,strlen(prompt1));
	
	return listenfd;
}

/* 函数功能：服务器RETR命令的控制函数
 * 传入参数：
 * -controlfd：控制套接字
 * -param：文件名称
 * -is_PORT：是否为PORT连接形式
 * -PORT_ip：PORT连接的地址
 * -PORT_port：PORT连接的端口
 * -is_PASV：是否为PASV连接形式
 * -PASV_listenfd：PASV监听端口
 * 返回值：正确返回0,否则返回-1。
 */
int server_retr(int controlfd,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd)
{
	char prompt1[] = "425 Please use 'PORT' or 'PASV' first to open data connection.\n\r";
	char prompt2[] = "425 Can't open data connection.\n\r";
	char prompt3[] = "550 Requested file action not taken.\n\r";
	char prompt4[] = "150 File status okay; about to open data connection.\n\r";
	char prompt5[] = "451 Requested action aborted: local error in processing.\n\r";
	char prompt6[] = "426 Connection closed; transfer aborted.\n\r";
	char prompt7[] = "125 Data connection already open; transfer starting.\n\r";
	char prompt8[] = "250 Requested file action okay, completed.\n\r";
	char prompt9[] = "226 Closing data connection.\n\r";
	
	int datafd = -1;
	int filefd = -1;	
	FILE* fp = NULL;
	char data[MAX_SIZE];
	char filename[200];
	size_t num_read;
	fd_set rfds,wfds; //读写文件句柄
	struct timeval timeout={3,0}; //select等待3秒
	int maxfd = 0;
	
	// 打开文件
	strcpy(filename,FILE_ROOT);
	strcat(filename,param);								
	fp = fopen(filename, "rb"); 
	if (!fp)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}		
	send_data(controlfd,prompt4,strlen(prompt4));
	filefd = fileno(fp);		
	
	//建立数据连接
	if(is_PORT)
	{
		struct sockaddr_in addr;	
	
		//创建数据套接字
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			fclose(fp);
			return -1;
    	}

		//设置协议地址
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT_port);
		addr.sin_addr.s_addr = inet_addr(PORT_ip);

		// 在套接字上创建连接
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
       		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
       		send_data(controlfd,prompt2,strlen(prompt2));
       		close(datafd);
       		fclose(fp);
			return -1;
    	}
    	
    	send_data(controlfd,prompt7,strlen(prompt7));
    	
	}
	else if(is_PASV)
	{
		//设置6s超时
		struct timeval accept_timeout = {6,0};  
		if (setsockopt(PASV_listenfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&accept_timeout, sizeof(struct timeval)) < 0)  
		{  
			printf("Error setsockopt(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			close(PASV_listenfd);
			fclose(fp);
			return -1;  
		}  
		datafd = accept_socket(PASV_listenfd);
		if(datafd<0)
		{
			printf("Create datafd error.\n");
			send_data(controlfd,prompt2,strlen(prompt2));
			close(PASV_listenfd);
			fclose(fp);
			return -1;
		}
		send_data(controlfd,prompt7,strlen(prompt7));
	
	}
	else
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	
	//传输数据
   	do
   	{
   		//清空集合
   		FD_ZERO(&rfds);
   		FD_ZERO(&wfds);
   		//添加描述符 
		FD_SET(datafd,&wfds); 
		FD_SET(filefd,&rfds); 
		//描述符最大值加1 
		maxfd=datafd>filefd?datafd+1:filefd+1; 
			
		//使用select判断状态
		switch(select(maxfd,&rfds,&wfds,NULL,&timeout))
		{
			case -1:
				send_data(controlfd,prompt5,strlen(prompt5));
				close(datafd);
				fclose(fp);
				return -1;
				break;
			case 0:
				break;
			default:
				//如果文件可读
				if(FD_ISSET(filefd, &rfds))
    			{
    				//读文件内容
					num_read = fread(data, 1, MAX_SIZE, fp);
					printf("num_read:%d\n",num_read);
					if (num_read < 0) 
					{
						send_data(controlfd,prompt5,strlen(prompt5));
						close(datafd);
						fclose(fp);
						return -1;
					}
						
					//如果接口可写
					if(FD_ISSET(datafd, &wfds))
					{
						//写入接口
						if (send_data(datafd, data, num_read) < 0) 
						{
							send_data(controlfd,prompt6,strlen(prompt6));
							close(datafd);
							fclose(fp);
							return -1;
						}
					}
   				}
    				
				break;
		}
   	}while(num_read > 0);
    	
   	send_data(controlfd,prompt8,strlen(prompt8));
   	
   	close(datafd);
   	
   	send_data(controlfd,prompt9,strlen(prompt9));
   	
   	fclose(fp);
	
	return 0;
}

int main(int argc, char **argv) {

	//开启服务器活动
	server_work(argc,argv);
	
	return 0;
}

