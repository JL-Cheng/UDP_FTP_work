#include "server.h"
/* =============================================
 * ����������
 * =============================================
 */
 
/* �������ܣ������׽��ֲ�����
 * ���������
 * -port���˿ں�
 * ����ֵ�����󷵻�-1;��ȷ�򷵻��׽�����������
 */
int create_socket(int port)
{	
	int sockfd;
	int flag = 1;
	struct sockaddr_in addr;

	//�����׽���
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//�����׽��ֵ�ַ����״̬
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
	
	//���׽��ֵ�ַ
	if(bind(sockfd,(struct sockaddr *) &addr,sizeof(addr)) < 0)
	{
		close(sockfd);
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//��ʼ����socket
	if (listen(sockfd, 10) < 0) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	return sockfd;
	
}

/* �������ܣ������׽��ֽ�������
 * ���������
 * -listenfd�����ڼ������׽���
 * ����ֵ�����󷵻�-1;��ȷ�򷵻��µ��׽�����������
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

/* �������ܣ����տͻ��˵��������
 * ���������
 * -sockfd����Ϣ������׽���
 * -cmd���ͻ�������
 * -param���ͻ����������
 * ����ֵ�����󷵻�-1;��ȷ�򷵻�0��
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
	
	//�ж�cmd�ĺϷ���
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
	
	//�ж�param�Ϸ���
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

/* �������ܣ���������
 * ���������
 * -sockfd�����ݴ�����׽���
 * -buf��������ָ��
 * -bufsize����������С
 * ����ֵ�����󷵻�-1;��ȷ�򷵻ط������ݵ��ֽ�����
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
 * ��������غ���
 * =============================================
 */

/* �������ܣ����������е���Ҫ���ƺ���
 * ���������
 * -argc������̨��������
 * -argv������̨����
 * ����ֵ��0
 */
int server_work(int argc,char **argv)
{
	int listenfd,controlfd;
	int pid = 1;
	
	//�������̨�������
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
	
	// ���������׽��� 
	if ((listenfd = create_socket(PORT)) < 0 )
	{
		printf("Error create socket!");
		exit(1);
	}
	
	//ʹ�ö���̽��ܶ���ͻ��˵�����
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

/* �������ܣ���������ÿ���ͻ��˵Ľ�������
 * ���������
 * -controlfd�����ƽ����׽���
 * ����ֵ����
 */
void server_process(int controlfd)
{
	int datafd;//���ݴ����׽���
	char cmd[5];//�ͻ������verb��
	char param[MAX_SIZE];//�ͻ����������
	
	char prompt1[] = "220 Anonymous FTP server ready.\n\r";

	//���ȷ��سɹ������ź�
	printf("success_connect\n");
	send_data(controlfd,prompt1,strlen(prompt1));
	
	//�����û���¼��֤
	printf("user_login\n");
	server_login(controlfd);

}

/* �������ܣ������û������¼
 * ���������
 * -controlfd�����ƽ����׽���
 * ����ֵ����
 */
void server_login(int controlfd)
{
	char cmd[5];//�ͻ������verb��
	char param[MAX_SIZE];//�ͻ����������
	int user_ok = 0;//�û����Ƿ���ȷ
	
	char prompt1[] = "530 Please use 'USER' command to login first.\n\r";
	char prompt2[] = "332 Please enter correct username.\n\r";
	char prompt3[] = "331 User name is ok, send your email address as password.\n\r";
	char prompt4[] = "230 Guest login ok, now you can do what you want.\n\r";
	
	while(1)
	{
		if(receive_cmd(controlfd,cmd,param))
			continue;
		//���ȱ�����USERָ��
		if(!(strcmp(cmd,CLIENT_INSTRUCTION[0])) && !user_ok)
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
		//����������䣬�û���¼�ɹ�
		else if(!(strcmp(cmd,CLIENT_INSTRUCTION[1])) && user_ok)
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

int main(int argc, char **argv) {

	//�����������
	server_work(argc,argv);
	
	return 0;
}

