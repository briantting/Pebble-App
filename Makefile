CC = clang

all: clean server

server: server.c start_server.c read_temperature.c
	$(CC) -o server server.c start_server.c read_temperature.c
	./server $(SERVER)

temp:
	$(CC) -o temperature read_temperature.c
	./temperature

clean: 
	rm -rf server *.o
