CC = clang

all: clean
	$(CC) -g -o exe main.c listen-to-arduino.c listen-to-pebble.c server.c queue.c
	#valgrind --leak-check=full ./exe $(SERVER)
	./exe $(SERVER)

test:
	$(CC) -g -o test server_test.c
	./test $(HOST) $(PORT)

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

.PHONY: all test clean
