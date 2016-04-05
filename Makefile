CC = clang

all: clean server

server: 
	$(CC) -o server main.c server.c read-temperature.c listen-to-arduino.c listen-to-pebble.c queue.c
	./server $(SERVER)

temp:
	$(CC) -o temperature read-temperature.c
	./temperature

heap: clean
	$(CC) -g -o heap-tester heap-tester.c heap.c
	valgrind --leak-check=full ./heap-tester

q: clean
	$(CC) -g -o q-tester q-tester.c queue.c
	valgrind --leak-check=full ./q-tester


clean: 
	rm -rf server temperature heap-tester *.o
