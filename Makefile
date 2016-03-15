CC = clang

all: clean server

server:
	$(CC) -o server server-threads.c
	./server $(SERVER)

clean: 
	rm -rf server *.o
