/*===============================================
*   文件名称：tcp_server.c
*   创 建 者：
*   创建日期：2021年09月01日
*   描    述：
================================================*/
#include "pro.h"

int main()
{ 
	signal(SIGCHLD, CHLD_handler);
	bzero(&HIS, sizeof(HIS));
	sqlite3 *db = init_database();
    int sockfd = init_server("0.0.0.0",6666,1024);
    if(-1 == sockfd)
    {
        printf("init_server error\n");
        return -1;
    }
	
    printf("listen....\n");
    
    char buf[SIZE] = {0};
    int ret;
    int i=0;
	struct sockaddr_in caddr;//客户端的地址结构
	bzero(&caddr, sizeof(caddr));
	
	socklen_t clen = sizeof(caddr);
    while(1)
    {
		int rws = accept(sockfd, (struct sockaddr *)&caddr, &clen);//阻塞等待客户端连接
		   if(-1 == rws)
		   {
			   perror("accept");
			   continue;
		   }
           printf("%s : %u\n",inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));//打印客户端的ip和port
           pid_t pid = fork();
		   if(0 > pid)
		   	{
		   		perror("fork");
				return -1;
		   	}
		   else if(0 == pid)
		   	{
		   		close(sockfd);
				char recv_buf[SIZE];
				while(1)
				{
					
					INFO *user = (INFO *)malloc(sizeof(INFO));
					bzero(user, sizeof(INFO));
					bzero(recv_buf, sizeof(recv_buf));
					ret = recv(rws,recv_buf,sizeof(recv_buf),0);//读客户端发送的消息
				    if(-1 == ret)
				    {
				        perror("recv");
				        close(rws);
						break;
				    }
				    else if(0 == ret)//客户端关闭
				    {
				        printf("client closed\n");
				        close(rws);
						break;
				    }
					user = (INFO *)recv_buf;
					#if ME_DEBUG
					printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
					#endif
					switch(user->type)
					{
						case 'R':
							memset(buf, 0, SIZE);
							ret = insert_table(db, user);
							printf("user->status = %d\n", user->status);
							if(user->status == 2)
							{
								sprintf(buf, "this usrname existed!");
							}
							else
							{
								sprintf(buf, "register success!");
							}
							ret = send(rws,buf,sizeof(buf),0);//给服务器发送消息
						    if(-1 == ret)
						    {
						        perror("send");
						        close(rws);
								break;
						    }
							break;
						case 'L':
							select_table(db, user);
							memset(buf, 0, SIZE);
							if(user->status == 2)
							{
								sprintf(buf, "login success!");
							}
							else
							{
								sprintf(buf, "login failed!");
							}
							ret = send(rws,buf,strlen(buf),0);//给服务器发送消息
						    if(-1 == ret)
						    {
						        perror("send");
						        close(rws);
								break;
						    }
							break;
					/*
						case 'F':
							select_brose_user(db);
							
							memset(buf, 0, SIZE);
							if(user->status == 2)
							{
								sprintf(buf, "search success!");
							}
							else
							{
								sprintf(buf, "search failed!");
							}
							ret = send(rws,buf,strlen(buf),0);//给服务器发送消息
						    if(-1 == ret)
						    {
						        perror("send");
						        close(rws);
								break;
						    }
						    */
							break;
						case 'G':
							translation(db, user, rws);
							break;
						case 'H':
							histroy(db, rws);
							break;
						default:
							break;
					}
				    
				}
				return -1;
			}
		   else
		   	{
		   		close(rws);
		   	}	

    }
    close(sockfd);

    return 0;
} 
