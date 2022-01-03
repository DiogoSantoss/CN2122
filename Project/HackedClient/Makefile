all: User

User: user.o log.o colors.o requestsUDP.o requestsTCP.o common.o
	gcc -o User user.o log.o colors.o requestsUDP.o requestsTCP.o common.o

clean:
	rm -f user.o log.o colors.o requestsUDP.o requestsTCP.o common.o User
