CC = clang

all: clean server

server: main.c server.c read_temperature.c
	$(CC) -o server main.c server.c read_temperature.c
	./server $(SERVER)

temp:
	$(CC) -o temperature read_temperature.c
	./temperature

heap: clean
	$(CC) -g -o heap-tester heap-tester.c heap.c
	valgrind --leak-check=full ./heap-tester

circ-q: clean
	$(CC) -g -o q-tester circ-q-tester.c circular-queue.c
	valgrind --leak-check=full ./q-tester


clean: 
	rm -rf server temperature heap-tester *.o
