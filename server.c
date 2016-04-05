#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#define BUFF_SIZE 10000


int start_server(void *argv_void)
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


  int reuse_addr = 1;
  if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,sizeof(int)) == -1) {
    perror("Setsockopt");
    exit(1);
  }



  // configure the server
  
  printf("error here\n");

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
	


  // 3. listen: indicates that we want to listen to the port to which we bound; 
  // second arg is number of allowed connections
  if (listen(sock, 5) == -1) { // 2nd arg is 5 in original
    perror("Listen");
    exit(1);
  }



  // once you get here, the server is set up and about to start listening
  printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
  fflush(stdout);

  return sock;

 
} 
