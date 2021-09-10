/*===============================================
*   文件名称：tcp_server.c
*   创 建 者：
*   创建日期：2021年09月01日
*   描    述：
================================================*/
#include "pro.h"

int main()
{ 
	int flag = 0;
    int sockfd = socket(AF_INET,SOCK_STREAM, 0 );//创建通信节点
    if(-1 == sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("sockfd=%d\n",sockfd);

	struct sockaddr_in saddr;//服务器的地址结构
    bzero(&saddr,sizeof(saddr));	
    saddr.sin_family = AF_INET;//ipv4协议
    saddr.sin_port = htons(6666);//5001~65535 端口号：主机字节序 -> 网络字节序
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");//ip地址：点分式 -> 二进制无符号网络字节序
    socklen_t slen = sizeof(saddr);

    int ret = connect(sockfd, (struct sockaddr *)&saddr,slen);//连接服务器
    if(-1 == ret)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }
    printf("connect success\n");
	INFO *user = malloc(sizeof(INFO));

    char buf[SIZE] = {0};
    do{

            bzero(buf,sizeof(buf));
            //读标准输入
            system("clear");
			if(!flag)
			{
				printf("\r\r*****************************\r\r\n");
				printf("\r\r1, register <username> <password>\r\n");
				printf("\r\r2, login <username> <password>\r\n");
				printf("\r\r*****************************\r\r\n");
			}
			else
			{
				printf("\r\r*****************************\r\r\n");
				printf("\r\r\ruser : %s\r\n",user->username);
				printf("\r\rusage : input word\r\n");
				printf("\r\r*****************************\r\r\n");
			}
            fgets(buf,sizeof(buf)-1, stdin);
            buf[strlen(buf)-1] = '\0';//处理'\n'
			if(!flag)
			{
				if(!strncmp(buf, "register", 8) && !flag)
				{
					bzero(user,sizeof(INFO));
					sscanf(buf, "%*s%s%s",user->username, user->password);
					user->type = 'R';
					user->status = -1;
					#if ME_DEBUG
					printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
					#endif
					bzero(buf, sizeof(buf));
					memcpy(buf, user, sizeof(INFO));
					ret = send(sockfd,buf,sizeof(buf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("send");
						close(sockfd);
		                break;
		            }
					memset(buf, 0, sizeof(buf));
					ret = recv(sockfd,buf,sizeof(buf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("recv");
						close(sockfd);
		                break;
		            }
					printf("read = %s\n",buf);
				}
				else if(!strncmp(buf, "login", 5) && !flag)
				{
					bzero(user,sizeof(INFO));
					sscanf(buf, "%*s%s%s",user->username, user->password);
					user->type = 'L';
					user->status = -1;
					#if ME_DEBUG
					printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
					#endif
					bzero(buf, sizeof(buf));
					memcpy(buf, user, sizeof(INFO));
					ret = send(sockfd,buf,sizeof(buf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("send");
						close(sockfd);
		                break;
		            }
					memset(buf, 0, sizeof(buf));
					ret = recv(sockfd,buf,sizeof(buf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("recv");
						close(sockfd);
		                break;
		            }
					printf("read = %s\n",buf);
					if(!strncmp(buf, "login success", 13))
					{
						flag = 1;
					}
				}
	    	}
			else
			{
				if(!strcmp(buf, "H"))
				{
					user->type = 'H';
					bzero(buf, sizeof(buf));
					memcpy(buf, user, sizeof(INFO));
					ret = send(sockfd,buf,sizeof(buf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("send");
						close(sockfd);
		                break;
		            }
					char newbuf[1480];
					memset(newbuf, 0, sizeof(newbuf));
					ret = recv(sockfd,newbuf,sizeof(newbuf),0);//给服务器发送消息
		            if(-1 == ret)
		            {
		                perror("recv");
						close(sockfd);
		                break;
		            }
					printf("%s\n",newbuf);
					getchar();
					continue;
				}
				user->type = 'G';
				strcpy(user->word,buf);
				#if ME_DEBUG
				printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
				#endif
				bzero(buf, sizeof(buf));
				memcpy(buf, user, sizeof(INFO));
				ret = send(sockfd,buf,sizeof(buf),0);//给服务器发送消息
	            if(-1 == ret)
	            {
	                perror("send");
					close(sockfd);
	                break;
	            }
				memset(buf, 0, sizeof(buf));
				ret = recv(sockfd,buf,sizeof(buf),0);//给服务器发送消息
	            if(-1 == ret)
	            {
	                perror("recv");
					close(sockfd);
	                break;
	            }
				printf("%s\n",buf);
			}
			getchar();
			
    }while(strncmp(buf,"quit",4) != 0);

    close(sockfd);

    return 0;
} 
