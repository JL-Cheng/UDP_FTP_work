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
		else if(!(strcmp(cmd,"RETR")))//"RETR"
		{
			int flag = client_retr(sockfd,buffer,param,is_PORT,PORT_ip,PORT_port,is_PASV, PASV_ip,PASV_port);
			if(flag < 0)
			{
				close(sockfd);
				return 1;
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
				memset(PASV_ip, 0, 20);
				PASV_port = 0;	
			}
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

/* 函数功能：客户端RETR命令控制函数
 * 传入参数：
 * -sockfd：控制套接字
 * -buffer：发送缓冲区内容
 * -param：文件名
 * -is_PORT：是否为PORT连接形式
 * -PORT_ip：PORT连接的地址
 * -PORT_port：PORT连接的端口
 * -is_PASV：是否为PASV连接形式
 * -PASV_ip：PASV连接的地址
 * -PASV_port：PASV连接的端口
 * 返回值：正确返回0,否则连接错误返回-1，参数错误返回1。
 */
int client_retr(int sockfd,char *buffer,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port)
{
	if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}

	if(is_PORT)
	{
		char data[MAX_SIZE];
		int size;
		int len;
		char *ch,*prompt[9];
		FILE* fp = fopen(param, "wb");	
		int datafd = -1;
		int filefd = -1;
		int listenfd = -1;	
		int back = 0;
		fd_set rfds,wfds; //读写文件句柄
		struct timeval timeout={3,0}; //select等待3秒
		int maxfd = 0;
		
		filefd = fileno(fp);	
		listenfd = create_socket(PORT_port);	
		if(listenfd<0)
		{
			printf("Create listenfd error.\n");
			close(sockfd);
			fclose(fp);
			return -1;
		}
		datafd = accept_socket(listenfd);
		if(datafd<0)
		{
			printf("Create datafd error.\n");
			close(listenfd);
			close(sockfd);
			fclose(fp);
			return -1;
		}
		close(listenfd);
		
		/*while(1)
		{
			if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
			{
				printf("Error recv(): %s(%d)\n", strerror(errno), errno);
				close(datafd);
				close(sockfd);
				fclose(fp);
				return -1;
			}
		
			buffer[len]='\0';			
			printf(">>>> %s\n", buffer);
			
			ch = strtok(buffer, " ");
			if(!strcmp(ch,"425")||!strcmp(ch,"550"))
			{
				close(datafd);
				fclose(fp);
				return 1;
			}
			else if(!strcmp(ch,"125"))
			{
				break;
			}
		}*/

    	
    	while(1)
    	{
    		//清空集合
    		FD_ZERO(&rfds);
    		FD_ZERO(&wfds);
    		//添加描述符 
			FD_SET(filefd,&wfds); 
			FD_SET(datafd,&rfds); 
			FD_SET(sockfd,&rfds); 
			//描述符最大值加1 
			maxfd=datafd>filefd?datafd+1:filefd+1; 
			maxfd=maxfd>(sockfd+1)?maxfd:(sockfd+1);
			
			//使用select判断状态
			switch(select(maxfd,&rfds,&wfds,NULL,&timeout))
			{
				case -1:
					fclose(fp);
					close(datafd);
					return 1;
					break;
				case 0:
					break;
				default:
					//如果控制套接字可读
					if(FD_ISSET(sockfd, &rfds))
    				{
    					//读套接字传来的内容
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							close(datafd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\n\r", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\n\r", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								close(datafd);
								fclose(fp);
								return 1;
							}
							else if(!strcmp(ch,"226"))
							{
								back = 1;
							}

						}
						
    				}
					//如果数据套接字可读
					if(FD_ISSET(datafd, &rfds))
    				{
    					//读套接字传来的内容
						size = recv(datafd, data, MAX_SIZE, 0);
						if(size)
							printf("size:%d\n",size);
						if (size < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
						
						//如果文件可写
	  					if(FD_ISSET(filefd, &wfds))
						{
							//写入文件
							if (fwrite(data, 1, size, fp) < size) 
							{
								fclose(fp);
								close(datafd);
								return 1;
							}

						}

    				}
					break;
			}
			
			if(back && !size)
			{
				close(datafd);
				fclose(fp);
				return 0;
			}
    	}
    	
    
	}
	else if(is_PASV)
	{
	}
	else
	{
		int len = 0;
		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return -1;
		}
		
		buffer[len]='\0';			
		printf(">>>> %s", buffer);
		return 1;
	}
}


int main(int argc, char **argv) 
{

	client_work(argc,argv);	

	return 0;
}
