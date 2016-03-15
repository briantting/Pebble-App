CC = clang

all: clean server

server:
	$(CC) -o server server.c
	./server $(SERVER)

clean: 
	rm -rf server *.o

