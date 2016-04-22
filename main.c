/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "listen-to-pebble.h"
#include "listen-to-arduino.h"
#include "server.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// track temperature data
float average = 0; 
float min, max;

int received; // flag for relaying Arduino's message confirmation
int arduino; // fd for arduino

int error() { 
  return -1;
}

int main(int argc, char *argv[])
{
  // check the number of arguments
  if (argc < 2) {
      printf("\nUsage: %s [port_number] \n", argv[0]);
      exit(-1);
  }

  int port_number = atoi(argv[1]);
  if (port_number <= 1024) {
    printf("\nPlease specify a port number greater than 1024\n");
    exit(-1);
  }

  puts("Enter 'q' at any time to quit.");
  pthread_t t1, t2;
  if (pthread_create(&t1, NULL, listen_to_arduino, argv) != 0) {
    return error(); 
  }
  if (pthread_create(&t2, NULL, listen_to_pebble, argv) != 0) {
    return error(); 
  } 
  if (pthread_join(t2, NULL) != 0) { 
    return error(); 
  } 
}

