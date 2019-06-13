#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"

int main(int argc,char *argv[])
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)return 1;

	struct sockaddr_in server_sock;
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_sock.sin_port = htons(SERVER_PORT);

	if(connect(sockfd,(struct sockaddr*)&server_sock,sizeof(server_sock)) < 0)
		return 2;
	printf("connect success...\n");

	char buf[1024];
	while(1){
		printf("client:#");
		fgets(buf,sizeof(buf),stdin);  //从标准输入得到一行数据，并且存入buf中
		buf[strlen(buf)-1] = '\0';

		write(sockfd,buf,sizeof(buf));//往sockfd中写入buf中的数据
//		printf("please wait...\n");

//		memset(buf,'\0',sizeof(buf));
//		read(sockfd,buf,sizeof(buf));//阻塞式读数据
//		printf("server:$ %s\n",buf);
	}
	close(sockfd);
	return 0;
}
