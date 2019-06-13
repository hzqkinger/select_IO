all:select_server client

select_server:select_server.c
	gcc -o $@ $^
client:client.c
	gcc -o $@ $^

.PHONY:clean
clean:
	rm -f select_server client
