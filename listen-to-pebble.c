// #include "read_temperature.h"
#include "server.h"
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
extern pthread_mutex_t lock;
char reply[100];
extern float max, min, average;


void* listen_to_pebble(void* argv) {
	
  int sock;
  struct sockaddr_in server_addr,client_addr; 
  sock = start_server(argv);

  int fd = -1;
  fd_set set;


  while(1) {

    // 4. accept: wait here until we get a connection on that port
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
    	int sin_size = sizeof(struct sockaddr_in);
      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1) {
      	printf("FD In: %d\n", fd);
        printf("Server got a connection from (%s, %d)\n",
            inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            
        // buffer to read data into
        char request[BUFF_SIZE];

        // 5. recv: read incoming message into buffer
        int bytes_received = recv(fd,request,1024,0);
        request[bytes_received] = '\0';
        // null-terminate the string
        printf("Here comes the message:\n");
        printf("%s\n", request);

        // 6. send: send the message over the socket
        // note that the second argument is a char*, and the third is the number of chars
        // char reply[BUFF_SIZE] = "{\n\"name\": \"";
        // trim newline char
        /*temperature = strsep(&temperature, "\n");*/
        
        char* begin_reply = "{\n\"name\": \"";
        char* end_reply = "\"\n }\n";
        bzero(reply, 100);

        strcat(reply, begin_reply);

        char *command = strdup(request);
      	char *token = strsep(&command, " ");
      	token = strsep(&command, " ");

        if(strlen(request) != 0) {
      	 	if(strncmp(request, "GET", 3) == 0) {
      	 		
      	 		if(strcmp(token, "/high") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%f", max);
      	 		} else if (strcmp(token, "/average") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%f", average);
      	 		} else if (strcmp(token, "/low") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%f", min);
      	 		}
      	 		 else {
      	 			strcat(reply, "Invalid GET request");
      	 		}
      	 		pthread_mutex_lock(&lock);
      	 		// reply = "Got a command";
      			// strcat(reply, temperature);
      			printf("FD Out: %d\n", fd);
      			pthread_mutex_unlock(&lock);
      	 	} else if(strncmp(request, "POST", 3) == 0) {
      	  	if(strcmp(token, "/change") == 0) {
      	 			strcat(reply, "Temp metric changed");
      	 		} else {
      	 			strcat(reply, "Invalid POST request");
      	 		}
      	  } 
      	 	else {
      	 		strcat(reply,"Only able to handle GET and POST requests");
      	 	}

        } else {
        	strcat(reply, "no message received from pebble");
        	
        }

        strcat(reply, end_reply);
        send(fd, reply, strlen(reply), 0); 

        

        // 7. close: close the connection
        close(fd);
      }
    }
  } // end while
  
  // 8. close: close the socket
  close(sock);
  printf("Server shutting down\n");

  return 0;
}
