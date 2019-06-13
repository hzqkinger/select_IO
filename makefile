all:client poll_server

client:client.c
	gcc -o $@ $^
poll_server:poll_server.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -f poll_server client
