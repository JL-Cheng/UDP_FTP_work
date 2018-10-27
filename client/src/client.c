#include"client.h"

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


/* �������ܣ��������ж�ȡ����
 * ���������
 * -buffer��������
 * -size����������С
 * -cmd������
 * -param���������
 * ����ֵ��������ȷ����0,���򷵻�-1.
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
	//�ж�cmd�ĺϷ���
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
	
	//�ж�param�Ϸ���
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
	char cmd[5];
	char param[MAX_SIZE];
	char *ch;
	int sockfd = -1;
	int len=0;
	
	char PORT_ip[20];//PORTָ�����ݴ���ip��ַ
	int PORT_port = 0;//PORTָ�����ݴ���˿�
	char PASV_ip[20];//PASVָ�����ݴ���ip��ַ
	int PASV_port = 0;//PASVָ�����ݴ���˿�
	int is_PORT = 0;//ʹ��PORT������������
	int is_PASV = 0;//ʹ��PASV���������

	//����socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
  
	//����Ŀ��������ip��port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(21);
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


	//���Ƚ��е�¼
	while(1)
	{
		//�������
		printf("fcp> ");
		if(get_input(buffer,MAX_SIZE,cmd,param))
			continue;
			
		//��������
		if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
		{
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
	
		//��������
		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		
		buffer[len]='\0';
			
		printf(">>>> %s", buffer);
		
		ch = strtok(buffer," ");
		if(!strcmp(ch,"230"))
			break;
	}
	
	//��ʼ��¼��Ĳ���
	while (1) 
	{
		//�������
		printf("fcp> ");
		if(get_input(buffer,MAX_SIZE,cmd,param))
			continue;

		//�ж�����
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
			else if(flag == 2)
				continue;
			
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
		else if(!(strcmp(cmd,"STOR")))//"STOR"
		{
			int flag = client_stor(sockfd,buffer,param,is_PORT,PORT_ip,PORT_port,is_PASV, PASV_ip,PASV_port);
			if(flag < 0)
			{
				close(sockfd);
				return 1;
			}
			else if(flag == 2)
				continue;
				
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
		else if(!(strcmp(cmd,"LIST")))//"LIST"
		{
			int flag = client_list(sockfd,buffer,is_PORT,PORT_ip,PORT_port,is_PASV, PASV_ip,PASV_port);
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
				
		//��������
		if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
		{
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		
		//��������
		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		
		buffer[len]='\0';
			
		printf(">>>> %s", buffer);
		
		ch = strtok(buffer," ");
		if(!strcmp(ch,"221"))
			break;
	} // ѭ���õ�������û�����
 
	close(sockfd);

	return 0;

}

/* �������ܣ��ͻ���PORT����Ŀ��ƺ���
 * ���������
 * -param��ip��ַ�Ͷ˿ںŵĲ���
 * -ip�������ip��ַ
 * -port������Ķ˿�
 * ����ֵ����������ȷ����0,���򷵻�-1��
 */
int client_port(char* param,char *ip,int *port)
{
	if(get_ip_port(param,ip,port)<0)
		return -1;
	return 0;
	
}

/* �������ܣ��ͻ���PASV����Ŀ��ƺ���
 * ���������
 * -sockfd�������׽���
 * -buffer��������Ϣ
 * -ip�������ip��ַ
 * -port������Ķ˿�
 * ����ֵ����������ȷ����0,���򷵻�-1��
 */
int client_pasv(int sockfd,char *buffer,char *ip,int *port)
{
	int len=0;
	char *ch;
	
	//��������
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
	if(strcmp(ch,"227"))
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

/* �������ܣ��ͻ���RETR������ƺ���
 * ���������
 * -sockfd�������׽���
 * -buffer�����ͻ���������
 * -param���ļ���
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_ip��PASV���ӵĵ�ַ
 * -PASV_port��PASV���ӵĶ˿�
 * ����ֵ����ȷ����0,�������Ӵ��󷵻�-1���������󷵻�1���ļ��򿪴��󷵻�2��
 */
int client_retr(int sockfd,char *buffer,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port)
{

	char data[MAX_SIZE];
	int data_size = -1;
	int len = 0;;
	char *ch,*prompt[9];
	FILE* fp;
	int datafd = -1;
	int filefd = -1;
	int receive_end = 0;
	int client_data_connect = 0;
	int server_data_connect = 0;	
	
	fd_set rfds,wfds; //��д�ļ����
	struct timeval timeout={3,0}; //select 3����Ѷ
	int maxfd = 0;
	
	//���ļ�
	ch = strtok(param,"/");
	while(ch)
	{
		param = ch;
		ch = strtok(NULL,"/");
	}
	fp = fopen(param, "wb");
	if (!fp)
	{
		printf("Open file error!\n");
		return 2;
	}	
	filefd = fileno(fp);
	
	if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		fclose(fp);
		return -1;
	}
	
	//�ж��Ƿ�������״̬
	if(!is_PORT && !is_PASV)
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
		fclose(fp);
		return 1;
	}

	//��PORT���ӽ�������
	if(is_PORT)
	{
		
		int listenfd = -1;		
		
		listenfd = create_socket(PORT_port);	
		if(listenfd<0)
		{
			printf("Create listenfd error.\n");
			close(sockfd);
			fclose(fp);
			return -1;
		}
		
		//select���нӿ��ж�
		while(1)
		{

    		//��ռ���
   			FD_ZERO(&rfds);
   			FD_ZERO(&wfds);
   			//��������� 
			FD_SET(filefd,&wfds); 
			FD_SET(sockfd,&rfds); 
			if (!client_data_connect)
			{
				FD_SET(listenfd,&rfds); 
				//���������ֵ��1 
				maxfd=listenfd>sockfd?listenfd+1:sockfd+1; 
				maxfd=maxfd>(filefd+1)?maxfd:(filefd+1);
			}
			if(client_data_connect && server_data_connect)
			{
				FD_SET(datafd,&rfds); 
				//���������ֵ��1 
				maxfd=datafd>filefd?datafd+1:filefd+1; 
				maxfd=maxfd>(sockfd+1)?maxfd:(sockfd+1);
			}	
			
			//ʹ��select�ж�״̬
			switch(select(maxfd,&rfds,&wfds,NULL,&timeout))
			{
				case -1:
					fclose(fp);
					if(datafd>0)
						close(datafd);
					if(listenfd>0)
						close(listenfd);
					return 1;
					break;
				case 0:
					break;
				default:
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							if(datafd>0)
								close(datafd);
							if(listenfd>0)
								close(listenfd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								if(datafd>0)
									close(datafd);
								if(listenfd>0)
									close(listenfd);
								fclose(fp);
								return 1;
							}
							else if(!strcmp(ch,"150"))
							{
								server_data_connect = 1;
							}
							else if(!strcmp(ch,"226"))
							{
								receive_end = 1;
							}

						}
						
					}
    				
					//��������׽��ֿɶ�
					if(!client_data_connect && FD_ISSET(listenfd, &rfds))
					{
						datafd = accept_socket(listenfd);
						if(datafd<0)
						{
							printf("Create datafd error.\n");
							close(listenfd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
						client_data_connect = 1;
						close(listenfd);
						listenfd = -1;
					}
					
					//��������׽��ֿɶ�
					if(server_data_connect && client_data_connect && FD_ISSET(datafd, &rfds))
	   				{
	   					//���׽��ִ���������
						data_size = recv(datafd, data, MAX_SIZE, 0);
						if (data_size < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
					
						//����ļ���д
	  					if(FD_ISSET(filefd, &wfds))
						{
							//д���ļ�
							if (fwrite(data, 1, data_size, fp) < data_size) 
							{
								fclose(fp);
								close(datafd);
								return 1;
							}

						}

	   				}
					break;
					
    			}
    			
    			if(receive_end && !data_size)
				{
					close(datafd);
					fclose(fp);
					return 0;
				}
		}    	
    
	}
	else
	{
	
		struct sockaddr_in addr;	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			fclose(fp);
			return -1;
		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PASV_port);
		addr.sin_addr.s_addr = inet_addr(PASV_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			close(datafd);
			fclose(fp);
			return -1;
		}
    	
		//select���нӿ��ж�
		while(1)
		{
			//��ռ���
   			FD_ZERO(&rfds);
   			FD_ZERO(&wfds);
   			//��������� 
			FD_SET(filefd,&wfds); 
			FD_SET(sockfd,&rfds); 
			FD_SET(datafd,&rfds); 
			//���������ֵ��1 
			maxfd=datafd>filefd?datafd+1:filefd+1; 
			maxfd=maxfd>(sockfd+1)?maxfd:(sockfd+1);
	
			
			//ʹ��select�ж�״̬
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
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
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
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								close(datafd);
								fclose(fp);
								return 1;
							}
							else if(!strcmp(ch,"226"))
							{
								receive_end = 1;
							}

						}
						
					}

					//��������׽��ֿɶ�
					if(FD_ISSET(datafd, &rfds))
	   				{
	   					//���׽��ִ���������
						data_size = recv(datafd, data, MAX_SIZE, 0);
						if (data_size < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
					
						//����ļ���д
	  					if(FD_ISSET(filefd, &wfds))
						{
							//д���ļ�
							if (fwrite(data, 1, data_size, fp) < data_size) 
							{
								fclose(fp);
								close(datafd);
								return 1;
							}

						}

	   				}
					break;
					
    			}
    			
    			if(receive_end && !data_size)
				{
					close(datafd);
					fclose(fp);
					return 0;
				}
		}    	
    		
	}
	
	return 0;
}

/* �������ܣ��ͻ���STOR������ƺ���
 * ���������
 * -sockfd�������׽���
 * -buffer�����ͻ���������
 * -param���ļ���
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_ip��PASV���ӵĵ�ַ
 * -PASV_port��PASV���ӵĶ˿�
 * ����ֵ����ȷ����0,�������Ӵ��󷵻�-1���������󷵻�1���ļ��򿪴��󷵻�2��
 */
int client_stor(int sockfd,char *buffer,char *param,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port)
{
	char data[MAX_SIZE];
	int data_size = -1;
	int len = 0;;
	char *ch,*prompt[9];
	FILE* fp;
	int datafd = -1;
	int filefd = -1;
	int send_end = 0;
	int client_data_connect = 0;
	int server_data_connect = 0;	
	
	fd_set rfds,wfds; //��д�ļ����
	struct timeval timeout={3,0}; //select 3����Ѷ
	int maxfd = 0;
	
	//���ļ�
	fp = fopen(param, "rb");
	if (!fp)
	{
		printf("Open file error!\n");
		return 2;
	}			
	filefd = fileno(fp);
	
	if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}
	
	//�ж��Ƿ�������״̬
	if(!is_PORT && !is_PASV)
	{
		int len = 0;
		if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
		{
			printf("Error recv(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			fclose(fp);
			return -1;
		}
		
		buffer[len]='\0';			
		printf(">>>> %s", buffer);
		return 1;
	}

	//��PORT���Ӵ�������
	if(is_PORT)
	{
		
		int listenfd = -1;		
		
		listenfd = create_socket(PORT_port);	
		if(listenfd<0)
		{
			printf("Create listenfd error.\n");
			close(sockfd);
			fclose(fp);
			return -1;
		}
		
		//select���нӿ��ж�
		while(1)
		{

			//��ռ���
   			FD_ZERO(&rfds);
   			FD_ZERO(&wfds);
   			//��������� 
			FD_SET(filefd,&rfds); 
			FD_SET(sockfd,&rfds); 
			if (!client_data_connect)
			{
				FD_SET(listenfd,&rfds); 
				//���������ֵ��1 
				maxfd=listenfd>sockfd?listenfd+1:sockfd+1; 
				maxfd=maxfd>(filefd+1)?maxfd:(filefd+1);
			}
			if(client_data_connect && server_data_connect && datafd>0)
			{
				FD_SET(datafd,&wfds); 
				//���������ֵ��1 
				maxfd=datafd>filefd?datafd+1:filefd+1; 
				maxfd=maxfd>(sockfd+1)?maxfd:(sockfd+1);
			}	
			
			//ʹ��select�ж�״̬
			switch(select(maxfd,&rfds,&wfds,NULL,&timeout))
			{
				case -1:
					fclose(fp);
					if(datafd>0)
						close(datafd);
					if(listenfd>0)
						close(listenfd);
					return 1;
					break;
				case 0:
					break;
				default:
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							if(datafd>0)
								close(datafd);
							if(listenfd>0)
								close(listenfd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								if(datafd>0)
									close(datafd);
								if(listenfd>0)
									close(listenfd);
								fclose(fp);
								return 1;
							}
							else if(!strcmp(ch,"150"))
							{
								server_data_connect = 1;
							}
							else if(!strcmp(ch,"226"))
							{
								send_end = 1;
							}

						}
						
					}
    				
					//��������׽��ֿɶ�
					if(!client_data_connect && FD_ISSET(listenfd, &rfds))
					{
						datafd = accept_socket(listenfd);
						if(datafd<0)
						{
							printf("Create datafd error.\n");
							close(listenfd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
						client_data_connect = 1;
						close(listenfd);
						listenfd = -1;
					}
					
						
					//����ӿڿ�д���ļ��ɶ�
					if(server_data_connect && client_data_connect && datafd > 0 &&
						FD_ISSET(datafd, &wfds) && FD_ISSET(filefd, &rfds))
					{

						//���ļ�����
						data_size = fread(data, 1, MAX_SIZE, fp);
					
						if (data_size < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
								
						//д��ӿ�
						if (send_data(datafd, data, data_size) < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
						
						//�������СΪ0
						if(!data_size)
						{
							close(datafd);
							datafd=-1;
						}
						
					}
	   	  				  				
					break;
					
    			}
    			
    			if(send_end)
    			{
    				fclose(fp);
					return 0;
				}
			
		}    	
    
	}
	else
	{
	
		struct sockaddr_in addr;	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			fclose(fp);
			return -1;
		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PASV_port);
		addr.sin_addr.s_addr = inet_addr(PASV_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			close(datafd);
			fclose(fp);
			return -1;
		}
    	
		//select���нӿ��ж�
		while(1)
		{
			//��ռ���
   			FD_ZERO(&rfds);
   			FD_ZERO(&wfds);
   			//��������� 
			FD_SET(filefd,&rfds); 
			FD_SET(sockfd,&rfds);
			if(datafd > 0) 
				FD_SET(datafd,&wfds); 
			//���������ֵ��1 
			maxfd=datafd>filefd?datafd+1:filefd+1; 
			maxfd=maxfd>(sockfd+1)?maxfd:(sockfd+1);
	
			
			//ʹ��select�ж�״̬
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
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							if(datafd>0)
								close(datafd);
							close(sockfd);
							fclose(fp);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								if(datafd>0)
									close(datafd);
								fclose(fp);
								return 1;
							}
							else if(!strcmp(ch,"226"))
							{
								send_end = 1;
							}

						}
						
					}
    					
					//����ӿڿ�д���ļ��ɶ�
					if(datafd > 0 && FD_ISSET(datafd, &wfds) && FD_ISSET(filefd, &rfds))
					{

						//���ļ�����
						data_size = fread(data, 1, MAX_SIZE, fp);
					
						if (data_size < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
								
						//д��ӿ�
						if (send_data(datafd, data, data_size) < 0) 
						{
							fclose(fp);
							close(datafd);
							return 1;
						}
						
						//�������СΪ0
						if(!data_size)
						{
							close(datafd);
							datafd=-1;
						}
						
					}
	   				
					break;
					
    			}
    			
    			if(send_end)
    			{
					fclose(fp);
					return 0;
				}
				
		}    	
    		
	}

	
	return 0;
}


/* �������ܣ��ͻ���LIST������ƺ���
 * ���������
 * -sockfd�������׽���
 * -buffer�����ͻ���������
 * -is_PORT���Ƿ�ΪPORT������ʽ
 * -PORT_ip��PORT���ӵĵ�ַ
 * -PORT_port��PORT���ӵĶ˿�
 * -is_PASV���Ƿ�ΪPASV������ʽ
 * -PASV_ip��PASV���ӵĵ�ַ
 * -PASV_port��PASV���ӵĶ˿�
 * ����ֵ����ȷ����0,�������Ӵ��󷵻�-1���������󷵻�1��
 */
int client_list(int sockfd,char *buffer,int is_PORT,char *PORT_ip,int PORT_port,int is_PASV,char * PASV_ip,int PASV_port)
{

	char data[MAX_SIZE];
	int data_size = -1;
	int len = 0;;
	char *ch,*prompt[9];
	int datafd = -1;
	int receive_end = 0;
	int client_data_connect = 0;
	int server_data_connect = 0;	
	
	fd_set rfds; //���ļ����
	struct timeval timeout={3,0}; //select 3����Ѷ
	int maxfd = 0;
	
	if (send(sockfd, buffer, (int)strlen(buffer), 0) < 0 )
	{
		printf("Error send(): %s(%d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}
	
	//�ж��Ƿ�������״̬
	if(!is_PORT && !is_PASV)
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

	//��PORT���ӽ�������
	if(is_PORT)
	{
		
		int listenfd = -1;		
		
		listenfd = create_socket(PORT_port);	
		if(listenfd<0)
		{
			printf("Create listenfd error.\n");
			close(sockfd);
			return -1;
		}
		
		//select���нӿ��ж�
		while(1)
		{

			//��ռ���
   			FD_ZERO(&rfds);
   			//��������� 
			FD_SET(sockfd,&rfds); 
			if (!client_data_connect)
			{
				FD_SET(listenfd,&rfds); 
				//���������ֵ��1 
				maxfd=listenfd>sockfd?listenfd+1:sockfd+1; 
			}
			if(client_data_connect && server_data_connect)
			{
				FD_SET(datafd,&rfds); 
				//���������ֵ��1 
				maxfd=datafd>sockfd?datafd+1:sockfd+1; 
			}	
			
			//ʹ��select�ж�״̬
			switch(select(maxfd,&rfds,NULL,NULL,&timeout))
			{
				case -1:
					if(datafd>0)
						close(datafd);
					if(listenfd>0)
						close(listenfd);
					return 1;
					break;
				case 0:
					break;
				default:
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							if(datafd>0)
								close(datafd);
							if(listenfd>0)
								close(listenfd);
							close(sockfd);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								if(datafd>0)
									close(datafd);
								if(listenfd>0)
									close(listenfd);
								return 1;
							}
							else if(!strcmp(ch,"150"))
							{
								server_data_connect = 1;
							}
							else if(!strcmp(ch,"226"))
							{
								receive_end = 1;
							}

						}
						
					}
    				
					//��������׽��ֿɶ�
					if(!client_data_connect && FD_ISSET(listenfd, &rfds))
					{
						datafd = accept_socket(listenfd);
						if(datafd<0)
						{
							printf("Create datafd error.\n");
							close(listenfd);
							close(sockfd);
							return -1;
						}
						client_data_connect = 1;
						close(listenfd);
						listenfd = -1;
					}
					
					//��������׽��ֿɶ�
					if(server_data_connect && client_data_connect && FD_ISSET(datafd, &rfds))
	   				{
	   					//���׽��ִ���������
						data_size = recv(datafd, data, MAX_SIZE, 0);
						if (data_size < 0) 
						{
							close(datafd);
							return 1;
						}
					
						if(data_size > 0)
							printf("%s",data);
	   				}
					break;
					
    			}
    			
    			if(receive_end && !data_size)
				{
					close(datafd);
					return 0;
				}
		}    	
    
	}
	else
	{
	
		struct sockaddr_in addr;	
		//���������׽���
		if ((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return -1;
		}

		//����Э���ַ
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PASV_port);
		addr.sin_addr.s_addr = inet_addr(PASV_ip);

		// ���׽����ϴ�������
		if(connect(datafd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		{
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			close(datafd);
			return -1;
		}
    	
		//select���нӿ��ж�
		while(1)
		{
			//��ռ���
   			FD_ZERO(&rfds);
   			//��������� 
			FD_SET(sockfd,&rfds); 
			FD_SET(datafd,&rfds); 
			//���������ֵ��1 
			maxfd=datafd>sockfd?datafd+1:sockfd+1; 
	
			
			//ʹ��select�ж�״̬
			switch(select(maxfd,&rfds,NULL,NULL,&timeout))
			{
				case -1:
					close(datafd);
					return 1;
					break;
				case 0:
					break;
				default:
					//��������׽��ֿɶ�
					if(FD_ISSET(sockfd, &rfds))
					{
						//���׽��ִ���������
						if ((len=recv(sockfd, buffer, MAX_SIZE, 0)) < 0) 
						{
							printf("Error recv(): %s(%d)\n", strerror(errno), errno);
							close(datafd);
							close(sockfd);
							return -1;
						}
		
						buffer[len]='\0';							
						
						prompt[0] = buffer;
						for(int i=0;;i++)
						{
							prompt[i] = strtok_r(prompt[i], "\r\n", &prompt[i+1]);
							if(!prompt[i])
								break;
							printf(">>>> %s\r\n", prompt[i]);
							
							ch = strtok(prompt[i]," ");
							if(!strcmp(ch,"451")||!strcmp(ch,"426")||!strcmp(ch,"425")||!strcmp(ch,"550"))
							{
								close(datafd);
								return 1;
							}
							else if(!strcmp(ch,"226"))
							{
								receive_end = 1;
							}

						}
						
					}

					//��������׽��ֿɶ�
					if(FD_ISSET(datafd, &rfds))
	   				{
	   					//���׽��ִ���������
						data_size = recv(datafd, data, MAX_SIZE, 0);
						if (data_size < 0) 
						{
							close(datafd);
							return 1;
						}
						if(data_size > 0)
							printf("%s",data);
	   				}
					break;
					
    			}
    			
    			if(receive_end && !data_size)
				{
					close(datafd);
					return 0;
				}
		}    	
    		
	}
	
	return 0;
}

int main(int argc, char **argv) 
{

	client_work(argc,argv);	

	return 0;
}
