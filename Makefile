CC = gcc
CFLAGS = -Wall -g -pedantic

all: mytalk
	
mytalk: main.o server.o client.o signal.o 
	$(CC) $(CFLAGS) -L ~pn-cs357/Given/Talk/lib64 -o mytalk main.o server.o client.o signal.o -ltalk -lncurses
		
main.o: main.c talk.h mytalk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/include main.c -o main.o
			
server.o: server.c talk.h mytalk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/include server.c -o server.o

client.o: client.c talk.h mytalk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/include client.c -o client.o

signal.o: signal.c
	$(CC) $(CFLAGS) -c signal.c -o signal.o	

clean:
	rm *.o

