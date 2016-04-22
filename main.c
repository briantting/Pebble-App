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

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// track temperature data
float average = 0; 
float min, max;

int received; // flag for relaying Arduino's message confirmation
int arduino; // fd for arduino
int sock; // fd for socket
struct sockaddr_in server_addr; // structs to represent the server


void start_server(int port_number)
{


  // 1. socket: creates a socket descriptor that you later use to make other system calls
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }

  // allow socket to forcibly bind to a port in use by another socket.
  int reuse_addr = 1;
  if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,sizeof(int)) == -1) {
    perror("Setsockopt");
    exit(1);
  }

  // configure the server
  server_addr.sin_port = htons(port_number); // specify port number
  server_addr.sin_family = AF_INET;           
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server_addr.sin_zero),8); 
}


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

  start_server(port_number);

  // DEBUG
  if(inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr)<=0) {
        perror("inet_pton");
        exit(1);
    } 

  int fd = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if (fd == -1) {
    perror("connect!!");
    exit(1);
  } 

  char* msg = "test";
  send(fd, msg, strlen(msg), 0);
  close(fd);
  // END DEBUG

  puts("Enter 'q' at any time to quit.");
  pthread_t t1, t2;

  if (pthread_create(&t1, NULL, listen_to_arduino, argv) != 0) {
    perror("pthread_create t1");
    return error(); 
  }
  /*if (pthread_create(&t2, NULL, listen_to_pebble, argv) != 0) {*/
    /*perror("pthread_create t2");*/
    /*return error(); */
  /*} */
  /*if (pthread_join(t2, NULL) != 0) { */
    /*perror("pthread_join t2");*/
    /*return error(); */
  /*} */
}

