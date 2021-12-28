all: Server

Server: server.o log.o colors.o requestsUDP.o common.o
	gcc -o Server server.o log.o colors.o requestsUDP.o common.o 

clean:
	rm -f server.o common.o log.o requestsUDP.o colors.o Server
