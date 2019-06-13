#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/epoll.h>
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
//	int opt = 1;
//	setsockopt(listen_sock,SOL_SOCKET,SOL_REUSEADDR,&opt,sizeof(opt));

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
void serverIO(int epfd,struct epoll_event revs[],int num,int listen_sock)  //能进入该函数的事件，都是已就绪的事件
{
	int i = 0;
	struct epoll_event ev;
	for(;i < num; ++i)
	{
		int sock = revs[i].data.fd;
		uint32_t events = revs[i].events;

		if(sock == listen_sock && (events & EPOLLIN)){
			//link event ready
			struct sockaddr_in client;
			socklen_t len = sizeof(&len);
			int new_sock = accept(sock,(struct sockaddr*)&client,&len);
			if(new_sock < 0){
				perror("accept");
				continue;
			}
			printf("get a new client\n");

			ev.events = EPOLLIN;
			ev.data.fd = new_sock;
			epoll_ctl(epfd,EPOLL_CTL_ADD,new_sock,&ev);
		}
		else if(events & EPOLLIN){
			//normal read event ready
			char buf[10240];
			ssize_t s = read(sock,buf,sizeof(buf)-1);
			if(s > 0){
				buf[s] = 0;
				printf("%s",buf);//read ok

				ev.events = EPOLLOUT;
				ev.data.fd = sock;
				epoll_ctl(epfd,EPOLL_CTL_MOD,sock,&ev);
			}
			else if(s == 0){
				printf("client quit ...!\n");
				close(sock);
				epoll_ctl(epfd,EPOLL_CTL_DEL,sock,NULL);
			}
			else perror("read");
		}
		else if(events & EPOLLOUT){
			//normal write event ready
			const char *echo_http = "HTTP/1.0 200 OK\r\n\r\n<html><h1>hello epoll server!</h1></html>\r\n";
			write(sock,echo_http,strlen(echo_http));
			close(sock);
			epoll_ctl(epfd,EPOLL_CTL_DEL,sock,NULL);
		}
		else{
			//bug
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

	int epfd = epoll_create(256);
	if(epfd < 0){
		perror("epoll_error");
		return 5;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	epoll_ctl(epfd,EPOLL_CTL_ADD,listen_sock,&ev);

	struct epoll_event revs[64];
	for(;;){
		int timeout = 1000;
		int num = epoll_wait(epfd,revs,sizeof(revs)/sizeof(revs[0]),timeout);
		switch(num)
		{
			case -1:
				perror("epoll_wait");
				break;
			case 0:
				printf("timeout\n");
				break;
			default:
				serverIO(epfd,revs,num,listen_sock);
				break;
		}

	}

	return 0;
}











