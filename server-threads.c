/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include "read_temp.h"
#define BUFF_SIZE 10000

char temperature [BUFF_SIZE] = "test";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void *start_server(void *argv_void)
{

  char** argv = (char**)argv_void;
  // structs to represent the server and client
  struct sockaddr_in server_addr,client_addr;    

  int sock; // socket descriptor

  // 1. socket: creates a socket descriptor that you later use to make other system calls
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }

  int temp = 1;
  if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
    perror("Setsockopt");
    exit(1);
  }

  // configure the server
  int PORT_NUMBER = atoi(argv[1]);
  server_addr.sin_port = htons(PORT_NUMBER); // specify port number
  server_addr.sin_family = AF_INET;         
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server_addr.sin_zero),8); 

  
  // 2. bind: use the socket and associate it with the port number
  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
    perror("Unable to bind");
    exit(1);
  }

  // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
  if (listen(sock, 1) == -1) { // 2nd arg is 5 in original
    perror("Listen");
    exit(1);
  }



  // once you get here, the server is set up and about to start listening
  printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
  fflush(stdout);

  int fd = -1;
  int successful_requests = 0;
  int bad_requests = 0;
  int bytes_received = 0;

  while(1) {

    // 4. accept: wait here until we get a connection on that port
    int sin_size = sizeof(struct sockaddr_in);
    fd_set set;
    FD_ZERO(&set);
    FD_SET(0, &set);
    FD_SET(sock, &set);
    if(select(sock+1, &set, NULL, NULL, NULL) == -1)
    {
        perror("select");
        exit(1);
    }
    int send_reply = 0;
    if (FD_ISSET(0, &set)) { 
      char buff [BUFF_SIZE]; 
      fgets(buff, sizeof(buff), stdin); 
      if (strcmp(buff, "q\n") == 0) {
        break; 
      }
    } else {

      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1 || send_reply) {
        printf("Server got a connection from (%s, %d)\n",
            inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            
        // buffer to read data into
        char request[BUFF_SIZE];

        // 5. recv: read incoming message into buffer
        int bytes_received = recv(fd,request,1024,0);
        // null-terminate the string
        printf("Here comes the message:\n");
        printf("%s\n", request);

        // 6. send: send the message over the socket
        // note that the second argument is a char*, and the third is the number of chars
        char reply [BUFF_SIZE] = "{\n\"name\": \"";
        strcat(reply, temperature);
        strcat(reply, "\"\n}\n");
        send(fd, reply, strlen(reply), 0); 
        puts(reply); 
      }
    }
  } // end while
  
  // 7. close: close the connection
  close(fd);
  printf("Server closed connection\n");

  // 8. close: close the socket
  close(sock);
  printf("Server shutting down\n");

  return 0;
} 

int error() { 
  return -1;
}

void* read_temp(void* arg) {
  /*const time_t now = time(0);*/
  /*strcpy(temperature, ctime(&now));*/
  return NULL;
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
  if ( pthread_create( &t2, NULL, read_temp, NULL  ) != 0  ) {
    return error(); 
  } 
  if ( pthread_join( t1, NULL  ) != 0  ) { 
    return error(); 
  } 
  if ( pthread_join( t2, NULL  ) != 0  ) { 
    return error();
  }
}

