#include"client.h"

/* =============================================
 * 基础处理函数
 * =============================================
 */

/* 函数功能：从命令行读取参数
 * 传入参数：
 * -buffer：缓冲区
 * -size：缓冲区大小
 * 返回值：输入正确返回0,否则返回-1.
 */
int get_input(char* buffer, int size)
{
	char *ch = 0;
	memset(buffer, 0, size);

	if(fgets(buffer, size, stdin))
	{
		ch = strchr(buffer, '\n');
		if (ch) 
			*ch = '\0';
	}
	
	if(strlen(buffer)==0)
		return -1;
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
    int res;
    int sockfd;
	int len=0;

	//创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  
	//设置目标主机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = 21;
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
		if(get_input(buffer,MAX_SIZE))
			continue;

		//发送命令
		if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
		{
			close(sockfd);
			return 1;
		}

		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
		
		buffer[len]='\0';
			
		printf(">>>> %s", buffer);

	} // 循环得到更多的用户输入
 
    close(sockfd);

	return 0;

}


int main(int argc, char **argv) 
{

	client_work(argc,argv);	

	return 0;
}
