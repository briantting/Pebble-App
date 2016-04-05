/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

/*#include <time.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "read_temperature.h"
#include "server.h"
#define BUFF_SIZE 10000

char temp_buff [BUFF_SIZE] = "test";
char* temperature = temp_buff;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
  /*start_server(port_number, argv[2]);*/
  if ( pthread_create( &t1, NULL, start_server, argv) != 0  ) {
    return error(); 
  }
  if ( pthread_create( &t2, NULL, read_temperature, NULL  ) != 0  ) {
    return error(); 
  } 
  if ( pthread_join( t1, NULL  ) != 0  ) { 
    return error(); 
  } 
}

