CC = clang

all: clean server

server: main.c start_server.c read_temperature.c
	$(CC) -o server main.c start_server.c read_temperature.c
	./server $(SERVER)

temp:
	$(CC) -o temperature read_temperature.c
	./temperature

heap: clean
	$(CC) -g -o heap-tester heap-tester.c heap.c
	valgrind --leak-check=full ./heap-tester

clean: 
	rm -rf server temperature heap-tester *.o
