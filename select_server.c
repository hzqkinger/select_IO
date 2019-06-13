#include<stdio.h>
#include<stdlib.h>
#include<sys/select.h>
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
	
	int opt = 1;
	setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//可以让服务器立即重启，

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
void serverIO(int *fd_list,int nfds,fd_set *read_fds)
{
	int i = 0;
	for(;i < nfds; ++i)
	{
		if(fd_list[i] == -1)continue;

		if(i == 0 && FD_ISSET(fd_list[i],read_fds)){  //处理listen_fd
			struct sockaddr_in client;     //我在前面逻辑中，总是把listen_fd放在数组的第0个位置,所以i==0时，一定就是listen_fd
			socklen_t len = sizeof(client);
			int connect_fd = accept(fd_list[i],(struct sockaddr*)&client,&len);//此时accept函数一定不会阻塞，因为一定有连接到来
			if(connect_fd < 0){
				perror("accept");
				continue;
			}
			printf("a new client connect\n");
			
			int j = 0;
			while(j < nfds && fd_list[j] != -1)++j;
			if(j < nfds)fd_list[j] = connect_fd;//获得新连接后，先把它放到fd_list中。（一定不要去读，因为可能没数据）
			else{printf("fd_list is full\n");close(connect_fd);}

			continue;
		}

		if(FD_ISSET(fd_list[i],read_fds)){  //处理connect_fd
			char buf[1024] = {0};
			ssize_t read_size = read(fd_list[i],buf,sizeof(buf) - 1);//到这里就可以读了，因为此时数据一定到来了
			if(read_size < 0){
				perror("read");
				close(fd_list[i]);
				fd_list[i] = -1;
				continue;
			}
			else if(read_size == 0){
				printf("client say goodbye\n");
				close(fd_list[i]);
				fd_list[i] = -1;
			}
			else{
				buf[read_size] = 0;
				printf("client say : %s\n",buf);
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

	int max_fd = listen_sock;
	fd_set read_fds;
	int fd_list[1024] = {-1}; //初始化fd_list数组
	int fd_list_size = sizeof(fd_list)/sizeof(fd_list[0]);
	fd_list[0] = listen_sock;
	int i = 1;
	for(;i < fd_list_size;++i){
		fd_list[i] = -1;
	}

	for(;;){
		FD_ZERO(&read_fds);
		for(i = 0;i < fd_list_size; ++i){                  //因为read_fds是一个输入输出型参数，所以每次循环进来都需要重新设置
			if(fd_list[i] != -1){
				FD_SET(fd_list[i],&read_fds);
				if(fd_list[i] > max_fd)
					max_fd = fd_list[i]; //顺便寻找最大的文件描述符
			}
		}

		struct timeval timeout = {1,0};//设定select的最后一个参数
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int ret = select(max_fd + 1,&read_fds,NULL,NULL,&timeout);//select仅仅只做一件事情，那就是IO中等待的环节
		switch(ret)
		{
			case -1:
				perror("select");
				break;
			case 0:
				printf("timeout\n");
				break;
			default:
				serverIO(fd_list,fd_list_size,&read_fds);
				break;
		}
	}

	return 0;
}
