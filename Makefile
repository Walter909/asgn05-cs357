CC = gcc
CFLAGS = -Wall -g -pedantic

all: mytalk
	
mytalk: main.o server.o client.o
	$(CC) $(CFLAGS) -L ~pn-cs357/Given/Talk/lib64 -o mytalk main.o server.o client.o -ltalk -lncurses
		
main.o: main.c talk.h mytalk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/includemain.c -o main.o
			
server.o: server.c talk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/include server.c -o server.o

client.o: client.c talk.h
	$(CC) $(CFLAGS) -c  -I ~pn-cs357/Given/Talk/include client.c -o client.o
					
clean:
	rm *.o

