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
	
	char prompt1[] = "220 FTP server ready.";
	
	//���ȷ��سɹ������ź�
	send_data(controlfd,prompt1,strlen(prompt1));
	
}

int main(int argc, char **argv) {

	//�����������
	server_work(argc,argv);
	
	return 0;
}

