all: Client Server

Client: Client/user.o Client/log.o Client/colors.o Client/requestsUDP.o Client/requestsTCP.o Client/common.o
	gcc -o user Client/user.o Client/log.o Client/colors.o Client/requestsUDP.o Client/requestsTCP.o Client/common.o

Server: Server/server.o Server/log.o Server/colors.o Server/requestsUDP.o Server/requestsTCP.o Server/common.o Server/directories.o
	gcc -o DS Server/server.o Server/log.o Server/colors.o Server/requestsUDP.o Server/requestsTCP.o Server/common.o Server/directories.o

clean:
	rm -f Server/server.o Server/common.o Server/log.o Server/requestsUDP.o Server/requestsTCP.o Server/colors.o Server/directories.o DS
	rm -f Client/user.o Client/log.o Client/colors.o Client/requestsUDP.o Client/requestsTCP.o Client/common.o user