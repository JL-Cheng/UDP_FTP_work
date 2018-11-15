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
	addr.sin_port = htons(port);
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
	if (listen(sockfd, 10) < 0) 
	{
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

	char prompt1[] = "500 Syntax error, command unrecognized.\r\n";
	char prompt2[] = "501 Syntax error in parameters.\r\n";

	memset(buffer, 0, MAX_SIZE);
	memset(cmd,0,5);
	memset(param,0,MAX_SIZE);

	num_bytes = recv(sockfd, buffer, MAX_SIZE, 0);
	if (num_bytes < 0)
	{
		printf("Error recv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	strtok(buffer,"\r\n");
	
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
	param_t = strtok(NULL, "\r");
	if(param_t && param_t[strlen(param_t)-1] == '\n')
		param_t[strlen(param_t)-1]='\0';
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

/* �������ܣ��õ�IP��ַ��port�˿ں�
 * ���������
 * -param������ip��port���ַ���
 * -ip�����ص�ip��ַ
 * -port�����صĶ˿ں�
 * ����ֵ�����󷵻�-1;��ȷ����0��
 */
int get_ip_port(char *param,char *ip,int *port)
{
	//������
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
	
	memset(FILE_ROOT,0,200);
	strcpy(FILE_ROOT,DEFAULT_ROOT);
	//�������̨�������
	for(int i=1;i<argc;i++)
	{
		if(!strcmp(argv[i],"-port"))
		{
			PORT = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i],"-root"))
		{
			strcpy(FILE_ROOT,argv[++i]);
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

/* �������ܣ���������ÿ���ͻ��˵Ľ�������
 * ���������
 * -controlfd�����ƽ����׽���
 * ����ֵ����
 */
void server_process(int controlfd)
{
	int PASV_listenfd = -1;//PASV�м��������׽���
	char PORT_ip[20];//PORTָ�����ݴ���ip��ַ
	int PORT_port = 0;//PORTָ�����ݴ���˿�
	int is_PORT = 0;//ʹ��PORT������������
	int is_PASV = 0;//ʹ��PASV���������
	char filename[200];//�ļ���
	int is_RNFR = 0;//�������ļ��������׶�
	
	int flag;//״̬����
	char cmd[5];//�ͻ������verb��
	char param[MAX_SIZE];//�ͻ����������
	
	int file_flag=0;//�ļ�λ��
	
	char prompt1[] = "220 Anonymous FTP server ready.\r\n";
	char prompt2[] = "502 Please use valid command.\r\n";
	char prompt3[] = "215 UNIX Type: L8\r\n";
	char prompt4[] = "221 Goodbye.\r\n";
	char prompt5[] = "350 Wait for next operation.\r\n";

	//���ȷ��سɹ������ź�
	printf("success_connect\n");
	send_data(controlfd,prompt1,strlen(prompt1));
	
	//�����û���¼��֤
	printf("user_login\n");
	server_login(controlfd);
	
	//����ѭ�������û���Ϣ
	printf("start_work\n");
	while(1)
	{
		if(receive_cmd(controlfd,cmd,param))
			continue;
		if(is_RNFR && strcmp(cmd,"RNTO"))
			is_RNFR = 0;
		//����PORTָ��
		if(!(strcmp(cmd,"PORT")))
		{
			flag = server_port(controlfd,param,PORT_ip,&PORT_port);
			if(flag < 0)//ʧ��
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
		//����PASVָ��
		else if(!(strcmp(cmd,"PASV")))
		{
			flag = server_pasv(controlfd);
			if(flag < 0)//ʧ��
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
		//����RETRָ��
		else if(!(strcmp(cmd,"RETR")))
		{
			if(server_retr(controlfd,param,is_PORT,PORT_ip,PORT_port,is_PASV,PASV_listenfd,file_flag))
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
			file_flag = 0;
			continue;
		}
		//����STORָ��
		else if(!(strcmp(cmd,"STOR")))
		{
			if(server_stor(controlfd,param,is_PORT,PORT_ip,PORT_port,is_PASV,PASV_listenfd))
			{
				printf("write file wrong.\n");
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
		//����LISTָ��
		else if(!(strcmp(cmd,"LIST")))
		{
			if(server_list(controlfd,is_PORT,PORT_ip,PORT_port,is_PASV,PASV_listenfd))
			{
				printf("Send list wrong.\n");
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
		//����RNFRָ��
		else if(!(strcmp(cmd,"RNFR")))
		{
			if(!server_rnfr(controlfd,param,filename))
				is_RNFR = 1;
			continue;
		}
		//����RNTOָ��
		else if(!(strcmp(cmd,"RNTO")))
		{
			server_rnto(controlfd,param,filename,is_RNFR);
			continue;
		}
		//����SYSTָ��
		else if(!(strcmp(cmd,"SYST")))
		{
			send_data(controlfd,prompt3,strlen(prompt3));
			continue;
		}	
		//����TYPEָ��
		else if(!(strcmp(cmd,"TYPE")))
		{
			server_type(controlfd,param);
			continue;
		}	
		//����QUITָ��
		else if(!(strcmp(cmd,"QUIT")))
		{
			send_data(controlfd,prompt4,strlen(prompt4));
			close(controlfd);
			return;
		}	
		//����MKDָ��
		else if(!(strcmp(cmd,"MKD")))
		{
			server_mkd(controlfd,param);
			continue;
		}	
		//����RMDָ��
		else if(!(strcmp(cmd,"RMD")))
		{
			server_rmd(controlfd,param);
			continue;
		}	
		//����CWDָ��
		else if(!(strcmp(cmd,"CWD")))
		{
			server_cwd(controlfd,param);
			continue;
		}
		//����PWDָ��
		else if(!(strcmp(cmd,"PWD")))
		{
			server_pwd(controlfd,param);
			continue;
		}
		//����RESTָ��
		else if(!(strcmp(cmd,"REST")))
		{
			send_data(controlfd,prompt5,strlen(prompt5));
			file_flag = atoi(param);
			continue;
		}
		//������������
		else
		{
			send_data(controlfd,prompt2,strlen(prompt2));
			continue;
		}
	}

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
	
	char prompt1[] = "530 Please use 'USER' command to login first.\r\n";
	char prompt2[] = "332 Please enter correct username.\r\n";
	char prompt3[] = "331 User name is ok, send your email address as password.\r\n";
	char prompt4[] = "230 Guest login ok, now you can do what you want.\r\n";
	
	while(1)
	{
		if(receive_cmd(controlfd,cmd,param))
			continue;
		
		//���ȱ�����USERָ��
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
		//����������䣬�û���¼�ɹ�
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

/* �������ܣ�������RNFR����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���ļ���ַ
 * -filename�����ļ���
 * ����ֵ���ɹ�����0,���򷵻�-1��
 */
int server_rnfr(int controlfd,char *param,char *filename)
{
	struct stat buf; 
	
	char prompt1[200];
	char prompt2[] = "450 Requested file action not taken.\r\n";
	
	//����·��
	memset(filename,0,200);
	strcpy(filename,FILE_ROOT);
	if(filename[strlen(filename)-1]!='/' && param[0]!='/')
		filename[strlen(filename)]='/';
	else if(filename[strlen(filename)-1]=='/' && param[0]=='/')
		filename[strlen(filename)-1]='\0';
	strcat(filename,param);		
	printf("filename:%s\n",filename);    
	 
	stat(filename, &buf); 
	if(S_IFREG & buf.st_mode)//���ļ�
	{ 
		sprintf(prompt1,"350 %s can be renamed.\r\n",filename);
		send_data(controlfd,prompt1,strlen(prompt1));
		return 0;
	}

	send_data(controlfd,prompt2,strlen(prompt2));
	return -1;
}

/* �������ܣ�������RNTO����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���ļ���ַ
 * -filename�����ļ���
 * -is_RNFR���Ƿ���RNFR����֮��
 * ����ֵ���ޡ�
 */
void server_rnto(int controlfd,char *param,char *filename,int is_RNFR)
{

	char prompt1[200];
	char prompt2[] = "503 Please use 'RNFR' first.\r\n";
	
	char new_filename[200];
	
	if(!is_RNFR)
	{
		send_data(controlfd,prompt2,strlen(prompt2));
 		return;
	}
	
	//���ļ�·��
	memset(new_filename,0,200);
	strcpy(new_filename,FILE_ROOT);
	if(new_filename[strlen(new_filename)-1]!='/' && param[0]!='/')
		new_filename[strlen(new_filename)]='/';
	else if(new_filename[strlen(new_filename)-1]=='/' && param[0]=='/')
		new_filename[strlen(new_filename)-1]='\0';
	strcat(new_filename,param);		
	printf("new_filename:%s\n",new_filename);
	
	if(!(rename(filename,new_filename)))
	{
		sprintf(prompt1,"250 %s is successfully renamed to %s.\r\n",filename,new_filename);
		send_data(controlfd,prompt1,strlen(prompt1));
		return;
	}
	else
	{
		sprintf(prompt1,"550 Renaming %s failed.\r\n",filename);
		send_data(controlfd,prompt1,strlen(prompt1));
		return;
	}
	
}

/* �������ܣ�������TYPE����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param��type�������
 * ����ֵ���ޡ�
 */
void server_type(int controlfd,char *param)
{
	char prompt1[] = "200 Type set to I.\r\n";
	char prompt2[] = "504 Command not implemented for that parameter.\r\n";
	if(!(strcmp(param,"I")))
	{
		send_data(controlfd,prompt1,strlen(prompt1));
	}
	else
	{
		send_data(controlfd,prompt2,strlen(prompt2));
	}
}

/* �������ܣ�������MKD����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���������
 * ����ֵ���ޡ�
 */
void server_mkd(int controlfd,char *param)
{
	char directory[200];
	int len = 0;
	struct stat buf; 
	
	char prompt1[300];
	char prompt2[] = "550 Make directory failed.\r\n";
	
	//����·��
	memset(directory,0,200);
	strcpy(directory,FILE_ROOT);
	if(directory[strlen(directory)-1]!='/' && param[0]!='/')
		directory[strlen(directory)]='/';
	else if(directory[strlen(directory)-1]=='/' && param[0]=='/')
		directory[strlen(directory)-1]='\0';
	strcat(directory,param);		
	printf("directory:%s\n",directory);
	  
	len=strlen(directory);
	for(int i=1; i<len; i++ )
	{
		if( directory[i]=='/' )
		{
			directory[i] = '\0';
			if(access(directory,0)!=0)
			{
				mkdir(directory, 0777);
			}
			directory[i]='/';
		}
	}
	if(len>0 && access(directory,0)!=0 )
	{
		mkdir(directory, 0777 );
	}
    
	 
	stat(directory, &buf ); 
	if(S_IFDIR & buf.st_mode)//��Ŀ¼
	{ 
		printf("folder\n"); 
		sprintf(prompt1,"257 %s Created.\r\n",directory);
		send_data(controlfd,prompt1,strlen(prompt1));
	}
	else if(S_IFREG & buf.st_mode)//���ļ�
	{ 
		printf("file\n"); 
		send_data(controlfd,prompt2,strlen(prompt2));
	} 
     
	return;

}

/* �������ܣ�������RMD����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���������
 * ����ֵ���ޡ�
 */
void server_rmd(int controlfd,char *param)
{
	char directory[200];
	
	char prompt1[300];
	char prompt2[] = "550 Remove directory failed.\r\n";
	
	//����·��
	memset(directory,0,200);
	strcpy(directory,FILE_ROOT);
	if(directory[strlen(directory)-1]!='/' && param[0]!='/')
		directory[strlen(directory)]='/';
	else if(directory[strlen(directory)-1]=='/' && param[0]=='/')
		directory[strlen(directory)-1]='\0';
	strcat(directory,param);		
	printf("directory:%s\n",directory);
	  
	if(!rmdir(directory))
	{
		sprintf(prompt1,"250 %s Removed.\r\n",directory);
		send_data(controlfd,prompt1,strlen(prompt1));
	}
	else
		send_data(controlfd,prompt2,strlen(prompt2));
     
	return;

}

/* �������ܣ�������CWD����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���������
 * ����ֵ���ޡ�
 */
void server_cwd(int controlfd,char *param)
{
	char directory[200];
	struct stat buf; 
	
	char prompt1[] = "250 Okay.\r\n";
	char prompt2[300];
	
	//����·��
	memset(directory,0,200);
	if(param[0]!='/')
		directory[0]='/';
	strcat(directory,param);		
	printf("directory:%s\n",directory);
	  
	if(access(directory,0)==0)
	{
		stat(directory, &buf);
		if(S_IFDIR & buf.st_mode)//��Ŀ¼
		{ 			
			memset(FILE_ROOT,0,200);
			strcpy(FILE_ROOT,directory);
			send_data(controlfd,prompt1,strlen(prompt1));
			return;
		}
	}
	
	sprintf(prompt2,"550 %s: No such file or directory.\r\n",directory);
	send_data(controlfd,prompt2,strlen(prompt2));  
	   
	return;
}

/* �������ܣ�������PWD����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���������
 * ����ֵ���ޡ�
 */
void server_pwd(int controlfd,char *param)
{

	char prompt1[300];
	sprintf(prompt1,"257 Current working directory:%s.\r\n",FILE_ROOT);
	send_data(controlfd,prompt1,strlen(prompt1));  
	   
	return;
}

/* �������ܣ�������PORT����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param��ip��ַ�Ͷ˿ںŵĲ���
 * -ip�������ip��ַ
 * -port������Ķ˿�
 * ����ֵ����������ȷ����0,���򷵻�-1��
 */
int server_port(int controlfd,char *param,char *ip,int *port)
{

	char prompt1[] = "501 Syntax error in parameters or arguments.\r\n";
	char prompt2[] = "200 PORT command is ok.\r\n";
	
	if(get_ip_port(param,ip,port)<0)
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	
	send_data(controlfd,prompt2,strlen(prompt2));
	printf("PORT:\nip:%s\nport:%d\n",ip,*port);
	return 0;
	
}

/* �������ܣ�������PASV����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * ����ֵ����ȷ����listenfd,���򷵻�-1��
 */
int server_pasv(int controlfd)
{
	char prompt1[50] = "227 Entering Passive Mode ";
	char prompt2[] = "421 Service not available, closing control connection.\r\n";
	
	int port = 0;
	char ip[20];
	char p[5];
	char *ch;
	
	//�������port
	srand((unsigned)time(NULL));
	port = 	rand() % 45535 + 20000;
	
	printf("PASV:\nport:%d\n",port);
	//��ʼ����
	int listenfd = create_socket(port);	
	if(listenfd<0)
	{
		printf("Create listenfd error.\n");
		send_data(controlfd,prompt2,strlen(prompt2));
		return -1;
	}
	
	struct sockaddr_in addr;
	socklen_t len = sizeof addr;
	getsockname(controlfd, (struct sockaddr*)&addr, &len); // ��÷�����ip
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
	strcat(prompt1,")\r\n");
	
	send_data(controlfd,prompt1,strlen(prompt1));
	
	return listenfd;
}

/* �������ܣ�������RETR����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���ļ�����
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_listenfd��PASV�����˿�
 * ����ֵ����ȷ����0,���򷵻�-1��
 */
int server_retr(int controlfd,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd,int file_flag)
{
	char prompt1[] = "425 Please use 'PORT' or 'PASV' first to open data connection.\r\n";
	char prompt2[] = "425 Can't open data connection.\r\n";
	char prompt3[] = "550 Requested file action not taken.\r\n";
	char prompt4[100];
	char prompt5[] = "451 Requested action aborted: local error in processing.\r\n";
	char prompt6[] = "426 Connection closed; transfer aborted.\r\n";
	char prompt7[] = "226 Closing data connection.\r\n";
	
	int datafd = -1;
	int filefd = -1;
	int start_write = 1;	
	FILE* fp = NULL;
	char data[MAX_SIZE];
	char temp[MAX_SIZE];
	char filename[200];
	int num_read = 0;
	fd_set rfds,wfds; //��д�ļ����
	struct timeval timeout={3,0}; //select�ȴ�3��
	int maxfd = 0;
	
	//��û�н�������
	if(!is_PORT && !is_PASV)
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	// ���ļ�
	memset(filename,0,200);
	strcpy(filename,FILE_ROOT);
	if(filename[strlen(filename)-1]!='/' && param[0]!='/')
		filename[strlen(filename)]='/';
	else if(filename[strlen(filename)-1]=='/' && param[0]=='/')
		filename[strlen(filename)-1]='\0';
	strcat(filename,param);		
	printf("filename:%s\n",filename);						
       
	fp = fopen(filename, "rb"); 
	if (!fp)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}		
	filefd = fileno(fp);
	printf("file_flag:%d\n",file_flag);
	if(lseek(filefd,file_flag,SEEK_SET)==-1)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}
			
        struct stat statbuf;
        stat(filename,&statbuf);
        int size=statbuf.st_size;
        printf("filesize:%d\n",size);
        sprintf(prompt4,"150 Data connection already open; transfer starting.The file's size is (%d).\r\n",size);

	
	//������������
	if(is_PORT)
	{
		struct sockaddr_in addr;	
	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			fclose(fp);
			return -1;
		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT_port);
		addr.sin_addr.s_addr = inet_addr(PORT_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			close(datafd);
			fclose(fp);
			return -1;
		}
    	
 		send_data(controlfd,prompt4,strlen(prompt4));
    	
	}
	else
	{
		//����6s��ʱ
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
		send_data(controlfd,prompt4,strlen(prompt4));
	
	}
	
	//��������
   	do
   	{
   		//��ռ���
   		FD_ZERO(&rfds);
   		FD_ZERO(&wfds);
   		//��������� 
		FD_SET(datafd,&wfds); 
		FD_SET(controlfd,&rfds);
		FD_SET(filefd,&rfds); 
		//���������ֵ��1 
		maxfd=datafd>filefd?datafd+1:filefd+1; 
		maxfd=maxfd>(controlfd+1)?maxfd:(controlfd+1);
			
		//ʹ��select�ж�״̬
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
				if(FD_ISSET(controlfd,&rfds))
				{
					memset(data,0,MAX_SIZE);
					num_read=recv(controlfd,data,MAX_SIZE,0);
					if(!strcmp(data,"finish"))
					{

						start_write=1;
					}
					printf("back:%s\n",data);
					memset(data,0,MAX_SIZE);
					num_read = 0;
				}
				//����ļ��ɶ�
				if(start_write&&FD_ISSET(filefd, &rfds))
				{
					//���ļ�����
					num_read = fread(temp, 1, MAX_SIZE-100, fp);
					printf("num_read:%d\n",num_read);
					if (num_read < 0) 
					{
						send_data(controlfd,prompt5,strlen(prompt5));
						close(datafd);
						fclose(fp);
						return -1;
					}
					sprintf(data, "%10d",num_read);
					printf("data:%s\n",data);
					strcat(data,temp);
					num_read=num_read+10;
						
					//����ӿڿ�д
					if(FD_ISSET(datafd, &wfds))
					{
						//д��ӿ�
						if (send_data(datafd, data, num_read) < 0) 
						{
							send_data(controlfd,prompt6,strlen(prompt6));
							close(datafd);
							fclose(fp);
							return -1;
						}
						start_write=0;
						memset(data,0,MAX_SIZE);
						memset(temp,0,MAX_SIZE);
					}
   				}
    				
				break;
		}
   	}while(num_read > 0);
   	
	send_data(controlfd,prompt7,strlen(prompt7));
	close(datafd);
	fclose(fp);
	
	return 0;
}

/* �������ܣ�������STOR����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -param���ļ�����
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_listenfd��PASV�����˿�
 * ����ֵ����ȷ����0,���򷵻�-1��
 */
int server_stor(int controlfd,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd)
{
	char prompt1[] = "425 Please use 'PORT' or 'PASV' first to open data connection.\r\n";
	char prompt2[] = "425 Can't open data connection.\r\n";
	char prompt3[] = "550 Requested file action not taken.\r\n";
	char prompt4[] = "150 Data connection already open; transfer starting.\r\n";
	char prompt5[] = "451 Requested action aborted: local error in processing.\r\n";
	char prompt6[] = "426 Connection closed; transfer aborted.\r\n";
	char prompt7[] = "226 Closing data connection.\r\n";
	
	int datafd = -1;
	int filefd = -1;	
	FILE* fp = NULL;
	char data[MAX_SIZE];
	char filename[200];
	int num_read = -1;
	char *ch;
	fd_set rfds,wfds; //��д�ļ����
	struct timeval timeout={3,0}; //select�ȴ�3��
	int maxfd = 0;
	
	//�������ӽ���
	if(!is_PORT && !is_PASV)
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	//���ļ�
	ch = strtok(param,"/");
	while(ch)
	{
		param = ch;
		ch = strtok(NULL,"/");
	}
	strcpy(filename,FILE_ROOT);
	if(filename[strlen(filename)-1]!='/')
		strcat(filename,"/");
	strcat(filename,param);		
	printf("filename:%s\n",filename);						
	fp = fopen(filename, "wb"); 
	if (!fp)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}		
	filefd = fileno(fp);		
	
	//������������
	if(is_PORT)
	{
		struct sockaddr_in addr;	
	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			fclose(fp);
			return -1;
		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT_port);
		addr.sin_addr.s_addr = inet_addr(PORT_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
	       		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
	       		send_data(controlfd,prompt2,strlen(prompt2));
	       		close(datafd);
	       		fclose(fp);
			return -1;
		}
    	
		send_data(controlfd,prompt4,strlen(prompt4));
    	
	}
	else
	{
		//����6s��ʱ
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
		send_data(controlfd,prompt4,strlen(prompt4));
	
	}
	
	//��������
   	do
   	{
   		//��ռ���
   		FD_ZERO(&rfds);
   		FD_ZERO(&wfds);
   		//��������� 
		FD_SET(datafd,&rfds); 
		FD_SET(filefd,&wfds); 
		//���������ֵ��1 
		maxfd=datafd>filefd?datafd+1:filefd+1; 
			
		//ʹ��select�ж�״̬
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
				//��������׽��ֿɶ�
				if(FD_ISSET(datafd, &rfds))
				{
					//���׽��ִ���������
					num_read = recv(datafd, data, MAX_SIZE, 0);
					printf("num_read:%d\n",num_read);
					if (num_read < 0) 
					{
						send_data(controlfd,prompt6,strlen(prompt6));
						close(datafd);
						fclose(fp);
						return -1;
					}
						
					//����ļ���д
					if(FD_ISSET(filefd, &wfds))
					{
						//д���ļ�
						if (fwrite(data, 1, num_read, fp) < num_read) 
						{
							send_data(controlfd,prompt5,strlen(prompt5));
							close(datafd);
							fclose(fp);
							return -1;
						}
					}

   				}
    				
				break;
		}
   	}while(num_read!=0);
    
   	send_data(controlfd,prompt7,strlen(prompt7));
   	close(datafd);
   	fclose(fp);
	
	return 0;	
}

/* �������ܣ�������LIST����Ŀ��ƺ���
 * ���������
 * -controlfd�������׽���
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_listenfd��PASV�����˿�
 * ����ֵ����ȷ����0,���򷵻�-1��
 */
int server_list(int controlfd,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,int PASV_listenfd)
{
	char prompt1[] = "425 Please use 'PORT' or 'PASV' first to open data connection.\r\n";
	char prompt2[] = "425 Can't open data connection.\r\n";
	char prompt3[] = "550 Requested file action not taken.\r\n";
	char prompt4[100];
	char prompt5[] = "451 Requested action aborted: local error in processing.\r\n";
	char prompt6[] = "426 Connection closed; transfer aborted.\r\n";
	char prompt7[] = "226 Closing data connection.\r\n";
	
	int datafd = -1;
	int filefd = -1;
	int start_write = 1;	
	FILE* fp = NULL;
	char data[MAX_SIZE];
	char sys_command[100];
	char filename[100];
	int num_read = 0;
	fd_set rfds,wfds; //��д�ļ����
	struct timeval timeout={3,0}; //select�ȴ�3��
	int maxfd = 0;
	
	//��û�н�������
	if(!is_PORT && !is_PASV)
	{
		send_data(controlfd,prompt1,strlen(prompt1));
		return -1;
	}
	//ִ��������ļ�
	sprintf(sys_command,"cd %s && ls -l > /tmp/tmp.txt",FILE_ROOT);
	if (system(sys_command) < 0)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}	
	strcpy(filename,"/tmp/tmp.txt");		
	printf("filename:%s\n",filename);						
	fp = fopen(filename, "rb"); 
	if (!fp)
	{
		send_data(controlfd,prompt3,strlen(prompt3));
		return -1;
	}		
	filefd = fileno(fp);	
        struct stat statbuf;
        stat(filename,&statbuf);
        int size=statbuf.st_size;
        printf("filesize:%d\n",size);
        sprintf(prompt4,"150 Data connection already open; transfer starting.The file's size is (%d).\r\n",size);	
	
	//������������
	if(is_PORT)
	{
		struct sockaddr_in addr;	
	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			fclose(fp);
			return -1;
 		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT_port);
		addr.sin_addr.s_addr = inet_addr(PORT_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			send_data(controlfd,prompt2,strlen(prompt2));
			close(datafd);
			fclose(fp);
			return -1;
		}
    	
 		send_data(controlfd,prompt4,strlen(prompt4));
    	
	}
	else
	{
		//����6s��ʱ
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
		send_data(controlfd,prompt4,strlen(prompt4));
	
	}
	
	//��������
   	do
   	{
   		//��ռ���
   		FD_ZERO(&rfds);
   		FD_ZERO(&wfds);
   		//��������� 
		FD_SET(datafd,&wfds); 
		FD_SET(controlfd,&rfds);
		FD_SET(filefd,&rfds); 
		//���������ֵ��1 
		maxfd=datafd>filefd?datafd+1:filefd+1; 
		maxfd=maxfd>(controlfd+1)?maxfd:(controlfd+1);
			
		//ʹ��select�ж�״̬
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
				if(FD_ISSET(controlfd,&rfds))
				{
					memset(data,0,MAX_SIZE);
					num_read=recv(controlfd,data,MAX_SIZE,0);
					if(!strcmp(data,"finish"))
					{

						start_write=1;
					}
					else if(!strcmp(data,"stop"))
					{
						close(datafd);
						fclose(fp);
						return -1;	
					}
					printf("back:%s\n",data);
					memset(data,0,MAX_SIZE);
					num_read = 0;
				}
				//����ļ��ɶ�
				if(start_write&&FD_ISSET(filefd, &rfds))
				{
					//���ļ�����
					num_read = fread(data, 1, MAX_SIZE, fp);
					printf("num_read:%d\n",num_read);
					if (num_read < 0) 
					{
						send_data(controlfd,prompt5,strlen(prompt5));
						close(datafd);
						fclose(fp);
						return -1;
					}
						
					//����ӿڿ�д
					if(FD_ISSET(datafd, &wfds))
					{
						//д��ӿ�
						if (send_data(datafd, data, num_read) < 0) 
						{
							send_data(controlfd,prompt6,strlen(prompt6));
							close(datafd);
							fclose(fp);
							return -1;
						}
						start_write=0;
					}
   				}
    				
				break;
		}
   	}while(num_read > 0);
   	
	send_data(controlfd,prompt7,strlen(prompt7));
	close(datafd);
	fclose(fp);
	
	return 0;
}

int main(int argc, char **argv) {

	//�����������
	server_work(argc,argv);
	
	return 0;
}

