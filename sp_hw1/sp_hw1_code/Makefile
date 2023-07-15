all: server.c
	gcc server.c -g -o write_server
	gcc server.c -g -D READ_SERVER -o read_server

clean:
	rm -f read_server write_server
