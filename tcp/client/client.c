#include"client.h"

/* =============================================
 * ����������
 * =============================================
 */

/* �������ܣ��������ж�ȡ����
 * ���������
 * -buffer��������
 * -size����������С
 * ����ֵ��������ȷ����0,���򷵻�-1.
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
 * �ͻ�����غ���
 * =============================================
 */

/* �������ܣ��ͻ������е���Ҫ���ƺ���
 * ���������
 * -argc������̨��������
 * -argv������̨����
 * ����ֵ����ȷ����0,���򷵻�1
 */
int client_work(int argc,char **argv)
{
	struct sockaddr_in addr;
	char buffer[MAX_SIZE];
    int res;
    int sockfd;
	int len=0;

	//����socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  
	//����Ŀ��������ip��port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = 21;
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) 
	{
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  

	//������Ŀ����������socket��Ŀ���������ӣ�-- ��������
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return 1;
	}
	//�������ӳɹ���Ϣ
	if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	buffer[len]='\0';
	printf(">>>> %s", buffer);


	while (1) 
	{
		//�������
		printf("fcp> ");
		if(get_input(buffer,MAX_SIZE))
			continue;

		//��������
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

	} // ѭ���õ�������û�����
 
    close(sockfd);

	return 0;

}


int main(int argc, char **argv) 
{

	client_work(argc,argv);	

	return 0;
}
