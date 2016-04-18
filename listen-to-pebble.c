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
#include <stdbool.h>
#define MSG_SIZE 300
extern pthread_mutex_t lock;
extern float max, min, average;
extern int arduino;
char reply[MSG_SIZE];
int sock; // socket descriptor

void start_server(void *argv_void)
{

  char** argv = (char**)argv_void;
  // structs to represent the server and client
  struct sockaddr_in server_addr,client_addr;    

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
}

/*
	Thread that starts a server and continously listens to Pebble Watch.
	Able to handle Pebble-Arduino temperature commands. 
	argv[1] should contain port to listen to. 
*/
void* listen_to_pebble(void* argv) {
	bool isCelsius = true; //handles which mode the temperature is in. Default is Celsius
  struct sockaddr_in server_addr,client_addr; 
  int fd = -1;
  fd_set set;

  /* 
  	Initializes the server.
  	Returns the location of the sock.
  */
	start_server(argv);

	/*
		Continously waits for Pebble message using select.
		Also listens to STDIN for "q" command to quit.
	*/
  while(1) {
    FD_ZERO(&set);
    FD_SET(0, &set);
    FD_SET(sock, &set);

    // 4. accept: wait here until we get a connection on that port
    if(select(sock+1, &set, NULL, NULL, NULL) == -1)
    {
        perror("select");
        exit(1);
    }

    if (FD_ISSET(0, &set)) { 
      char buff [MSG_SIZE]; 
      fgets(buff, sizeof(buff), stdin); 
      if (strcmp(buff, "q\n") == 0) {
        break; 
      }
    } else {
    	int sin_size = sizeof(struct sockaddr_in);
      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1) {
        printf("Server got a connection from (%s, %d)\n",
            inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            
        // buffer to read data into
        char request[MSG_SIZE];

        // 5. recv: read incoming message into buffer
        int bytes_received = recv(fd,request,1024,0);
        
        // null-terminate the string
        request[bytes_received] = '\0';
        
        printf("Here comes the message:\n");
        printf("%s\n", request);


        
        pthread_mutex_lock(&lock);
		    float temp_max = max;
		    float temp_avg = average;
		    float temp_min = min;
		    char *metric = "c ";
      	pthread_mutex_unlock(&lock);

      	//Handle conversions if mode is in Farenheit
		    if(!isCelsius) {
		    	temp_max = temp_max * 1.8 + 32;
		    	temp_min = temp_min * 1.8 + 32;
		    	temp_avg = temp_avg * 1.8 + 32;
		    	metric = "f ";
		    }

		    /*
		    	Creates a JSON reply for pebble.
		    	There must be a default reply, otherwise Pebble will keep pinging.
		    */
        char* begin_reply = "{\n\"name\": \"";
        char* end_reply = "\"\n }\n";
        bzero(reply, MSG_SIZE);

        strcat(reply, begin_reply);

        char *command = strdup(request);
      	char *token = strsep(&command, " ");
      	token = strsep(&command, " ");

        if(strlen(request) != 0) {
      	 	if(strncmp(request, "GET", 3) == 0) {
      	 		
      	 		if(strcmp(token, "/high") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%5.2f", temp_max);
      	 			strcat(reply, metric);
      	 		} else if (strcmp(token, "/average") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%5.2f", temp_avg);
      	 			strcat(reply, metric);
      	 		} else if (strcmp(token, "/low") == 0) {
      	 			snprintf(&reply[strlen(reply)], 80, "%5.2f", temp_min);
      	 			strcat(reply, metric);
      	 		}
      	 		 else {
      	 			strcat(reply, "Invalid GET request");
      	 		}
      	 		
      	 	} else if(strncmp(request, "POST", 3) == 0) {
      	  	if(strcmp(token, "/change") == 0) {
      	  		isCelsius = !isCelsius;
      	  		write(arduino, "1", 1);
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

        // 6. send: send the message over the socket
        // note that the second argument is a char*, and the third is the number of chars
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
