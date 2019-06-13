#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<poll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>

int startup(int port)
{
	int listen_sock = socket(AF_INET,SOCK_STREAM,0); //创建监听套接字
	if(listen_sock < 0){
		perror("socker");
		return 2;
	}
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listen_sock,(struct sockaddr*)&server,sizeof(server)) < 0){ //绑定端口号
		perror("bind");
		return 3;
	}
	if(listen(listen_sock,5) < 0){ //将套接字声明为监听状态，此时的套接字可监听是否有客户端来连接
		perror("listen");
		return 4;
	}
	return listen_sock;
}
void serverIO(struct pollfd *fd_list,int nfds,int listen_fd)
{
	int i = 0;
//	for(;i < nfds; ++i)
	for(;i < 10; ++i)
	{
		if(fd_list[i].fd == -1)continue;
		if(!(fd_list[i].revents & POLLIN))continue;

		if(i == 0 && fd_list[i].fd == listen_fd){  //处理listen_fd
			struct sockaddr_in client;     //我在前面逻辑中，总是把listen_fd放在数组的第0个位置,所以i==0时，一定就是listen_fd
			socklen_t len = sizeof(client);
			int connect_fd = accept(fd_list[i].fd,(struct sockaddr*)&client,&len);//此时accept函数一定不会阻塞，因为一定有连接到来
			if(connect_fd < 0){
				perror("accept");
				continue;
			}
			printf("a new client connect\n");
			
			int j = 0;
			while(j < nfds && fd_list[j].fd != -1)++j;
			if(j < nfds){fd_list[j].fd = connect_fd;fd_list[j].events = POLLIN;}//获得新连接后，先把它放到fd_list中。（一定不要去读，因为可能没数据）
		}
		else {  //处理connect_fd
			char buf[1024] = {0};
			ssize_t read_size = read(fd_list[i],buf,sizeof(buf) - 1);//到这里就可以读了，因为此时数据一定到来了
			if(read_size < 0){
				perror("read");
				continue;
			}
			else if(read_size == 0){
				printf("client say goodbye\n");
				close(fd_list[i]);
				fd_list[i].fd = -1;
			}
			else{
				buf[read_size] = 0;
				printf("client say : %s",buf);
			}
		}
	}
}
int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("Usage :%s [port]\n",argv[0]);
		return 1;
	}
	int listen_sock = startup(atoi(argv[1]));//创建，绑定，监听

	struct pollfd fd_list[1024];
	int i = 0;
	for(; i < 1024;++i){
		fd_list[i].fd = -1;
		fd_list[i].events = 0;
		fd_list[i].revents = 0;
	}
	fd_list[0].fd = listen_sock;
	fd_list[0].events = POLLIN;

	for(;;){
		int ret = poll(fd_list,1024,1000);
		if(ret < 0){
			perror("poll");
			continue;
		}
		if(ret == 0){
			printf("poll timeout\n");
			continue;
		}
		serverIO(fd_list,1024,listen_sock);

	}

	return 0;
}











