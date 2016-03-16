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

extern char* temperature;
extern pthread_mutex_t lock;

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

  // 3. listen: indicates that we want to listen to the port to which we bound; 
  // second arg is number of allowed connections
  if (listen(sock, 2) == -1) { // 2nd arg is 5 in original
    perror("Listen");
    exit(1);
  }



  // once you get here, the server is set up and about to start listening
  printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
  fflush(stdout);

  int fd = -1;
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

    if (FD_ISSET(0, &set)) { 
      char buff [BUFF_SIZE]; 
      fgets(buff, sizeof(buff), stdin); 
      if (strcmp(buff, "q\n") == 0) {
        break; 
      }
    } else {

      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1) {
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
        // trim newline char
        /*temperature = strsep(&temperature, "\n");*/
        pthread_mutex_lock(&lock);
        strcat(reply, temperature);
        pthread_mutex_unlock(&lock);

        strcat(reply, "\"\n}\n");
        /*char *reply = "{\n\"name\": \"cit595\"\n}\n";*/

        send(fd, reply, strlen(reply), 0); 
        puts(reply); 

        // 7. close: close the connection
        close(fd);
        printf("Server closed connection\n");
      }
    }
  } // end while
  
  // 8. close: close the socket
  close(sock);
  printf("Server shutting down\n");

  return 0;
} 
