all: Server

Server: server.o log.o colors.o requestsUDP.o requestsTCP.o common.o directories.o
	gcc -o DS server.o log.o colors.o requestsUDP.o requestsTCP.o common.o directories.o

clean:
	rm -f server.o common.o log.o requestsUDP.o requestsTCP.o colors.o directories.o DS
