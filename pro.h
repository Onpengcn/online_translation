#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <stdlib.h>
#include <sqlite3.h>
#define SIZE 128
#define ME_DEBUG 0
int trans_flag;

struct{
	int num;
	char buf[10][SIZE];
	char username[10][20];
}HIS;

typedef struct dict_info{
	int status;
	char type;
	char word[64];
	char username[20];
	char password[20];
}INFO;

int histroy(sqlite3 *db, int rws)
{
	char buf[1480];
	bzero(buf, sizeof(buf));
	for(int i=HIS.num-1; i>=0; i--)
	{
		strncat(buf, HIS.username[i],20);
		strncat(buf, "\n",1);
		strncat(buf, HIS.buf[i],128);
		strncat(buf, "\n",1);
		//sprintf(buf, "%s : %s", HIS.username[i], HIS.buf[i]);
		
	}
	int ret = send(rws,buf,sizeof(buf),0);//给服务器发送消息
    if(-1 == ret)
    {
        perror("send");
        close(rws);
		return -1;
    }
}

int translation_callback(void *para,int f_num, char **f_value, char **f_name)
{
	char buf[SIZE];
	bzero(buf, sizeof(buf));
	strcpy(buf, f_value[1]);
	int rws = *(int *)para;
	int ret = send(rws,buf,strlen(buf),0);//给服务器发送消息
    if(-1 == ret)
    {
        perror("send");
        close(rws);
		return -1;
    }
	trans_flag = 1;
	strcpy(HIS.buf[HIS.num], f_value[1]);
	return 0;
}

int translation(sqlite3 *db, INFO *user, int rws)
{
	trans_flag = 0;
	char buf[SIZE];
	char *errmsg;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "select * from dictionary where word='%s'", user->word);
	#if ME_DEBUG
	printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
	#endif
	if(SQLITE_OK != sqlite3_exec(db, buf, translation_callback, (void *)&rws, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return -1;
	}
	if(!trans_flag)
	{
		bzero(buf, sizeof(buf));
		strcpy(buf, "Word Not Found!");
		int ret = send(rws,buf,strlen(buf),0);//给服务器发送消息
	    if(-1 == ret)
	    {
	        perror("send");
	        close(rws);
			return -1;
	    }
		return 0;
	}
	else
	{
		strcpy(HIS.username[HIS.num], user->word);
		HIS.num++;
	}
	return 0;
}



int select_brose_callback(void *para,int f_num, char **f_value, char **f_name)
{
	for(int i=0;i<f_num;i++)
    {
            printf("%s : %s\n", f_name[i], f_value[i]);
    }
	return 0;
}

int select_table_callback(void *para,int f_num, char **f_value, char **f_name)
{
	INFO *user = (INFO *)para;
	user->status = 2;
	return 0;
}

int insert_table_callback(void *para,int f_num, char **f_value, char **f_name)
{
	INFO *user = (INFO *)para;
	if(!strcmp(f_value[0], user->username))
		user->status = 2;
	return 0;
}

int insert_table(sqlite3 *db, INFO *user)
{
	char buf[128];
	char *errmsg;
	user->status = 0;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "select * from usr");
	if(SQLITE_OK != sqlite3_exec(db, buf, insert_table_callback, (void *)user, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return -1;
	}
	if(2 == user->status)
		return 1;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "insert into usr values('%s','%s')", user->username, user->password);
	#if ME_DEBUG
	printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
	#endif
	if(SQLITE_OK != sqlite3_exec(db, buf, NULL, NULL, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return -1;
	}
	return 0;
}

int select_table(sqlite3 *db, INFO *user)
{
	char buf[SIZE];
	char *errmsg;
	user->status = -1;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "select * from usr where usrname='%s' and passwd='%s'", user->username, user->password);
	#if ME_DEBUG
	printf("%d %c %s, %s\n",user->status, user->type, user->username, user->password);
	#endif
	if(SQLITE_OK != sqlite3_exec(db, buf, select_table_callback, (void *)user, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return -1;
	}
	
	return 0;
}

int select_brose_user(sqlite3 *db)
{
	char buf[128];
	char *errmsg;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "select * from usr");
	if(SQLITE_OK != sqlite3_exec(db, buf, select_brose_callback, NULL, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return -1;
	}
	return 0;
}


sqlite3 *init_database()
{
	sqlite3 *db;
	char *errmsg;
	int ret = sqlite3_open("data.db", &db);
	if(0 > ret)
	{
		sqlite3_close(db);
	}
	if(SQLITE_OK != sqlite3_exec(db, "delete from usr", NULL, NULL, &errmsg))
	{
		printf("error: %s\n", errmsg);
		sqlite3_close(db);
		return NULL;
	}
	return db;
}


int init_server(char *ipaddr, unsigned short port, int backlog)
{
    int sockfd = socket(AF_INET,SOCK_STREAM, 0 );//创建通信节点
    if(-1 == sockfd)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in saddr;//服务器的地址结构
    bzero(&saddr,sizeof(saddr));

    saddr.sin_family = AF_INET;//ipv4协议
    saddr.sin_port = htons(port);//5001~65535 端口号：主机字节序 -> 网络字节序
    saddr.sin_addr.s_addr = inet_addr(ipaddr);//ip地址：点分式 -> 二进制无符号网络字节序
    socklen_t slen = sizeof(saddr);
	int on = 1;
		if(-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))//允许发送广播数据
		{
			perror("setsockopt");
			return -1;
		}

    int ret = bind(sockfd, (struct sockaddr *)&saddr,slen);//将服务器自己的ip地址和端口号与sockfd绑定
    if(-1 == ret)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }
    ret = listen(sockfd, backlog);//监听是否有客户端请求  sockfd变为监听套接字 
    if(-1 == ret)
    {
        perror("listen");
        close(sockfd);
        return -1;
    
    }

    return sockfd;
}

void CHLD_handler(int signo)
{
	if(signo == SIGCHLD)
	{
		while(0 < waitpid(-1,NULL,WNOHANG));
	}
}

