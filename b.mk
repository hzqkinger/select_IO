all:epoll_server client

epoll_server:epoll_server.c
	gcc -o $@ $^
client:client.c
	gcc -o $@ $^

.PHONY:clean
clean:
	rm -f epoll_server client
