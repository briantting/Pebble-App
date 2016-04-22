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
#include <time.h>
#define MSG_SIZE 300
extern pthread_mutex_t lock;
extern float max, min, average;
extern int arduino;
extern int sock; // socket descriptor
extern int received;
extern struct sockaddr_in server_addr;
int TEMP_LENGTH = 80;
double WAIT_TIME = 5;
char reply[MSG_SIZE];

int message_received() {
  time_t tick = time(NULL);
  while (!received && difftime(time(NULL), tick) < WAIT_TIME);
  int ret_val = received;
  pthread_mutex_lock(&lock);
  received = 0;
  pthread_mutex_unlock(&lock);
  return ret_val;
}

void start_server(void *argv_void)
{

  char** argv = (char**)argv_void;

  // structs to represent the server and client
  struct sockaddr_in client_addr;
  
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
  printf("\nServer configured to listen on port %d\n", atoi(argv[1]));
  
  fflush(stdout);
}

/*
  Thread that starts a server and continously listens to Pebble Watch.
  Able to handle Pebble-Arduino temperature commands. 
  argv[1] should contain port to listen to. 
*/
void* listen_to_pebble(void* argv) {
  bool isCelsius = true; //handles which mode the temperature is in. Default is Celsius
  /*struct sockaddr_in server_addr,client_addr; */
  struct sockaddr_in client_addr; 
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
      
      // FOR DEBUGGING ONLY
      int send_message = 0;
      switch (buff[0]) {
        case 'c': 
          puts("Send signal to change temperature units."); 
          send_message = 1;
          break;
        case 'a':
          puts("Send signal to arm alarm.");
          send_message = 1;
          break;
        case 'd':
          puts("Send signal to disarm alarm.");
          send_message = 1;
          break;
      }
      if (send_message) {
        char msg [2] = {buff[0], '\0'};
        write(arduino, msg, 1); 
        if (message_received()) {
          puts("Arduino received message.");
        } else {
          puts("No response from Arduino.");
        }
      }
    } else {
      int sin_size = sizeof(struct sockaddr_in);
      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1) {
        printf("\nServer got a connection from (%s, %d)\n",
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
            char* main_reply = reply + strlen(reply);

            if(strcmp(token, "/high") == 0) {
              snprintf(main_reply, TEMP_LENGTH, "%5.2f", temp_max);
              strcat(reply, metric);
            } else if (strcmp(token, "/average") == 0) {
              snprintf(main_reply, TEMP_LENGTH, "%5.2f", temp_avg);
              strcat(reply, metric);
            } else if (strcmp(token, "/low") == 0) {
              snprintf(main_reply, TEMP_LENGTH, "%5.2f", temp_min);
              strcat(reply, metric);
            } else if (strcmp(token, "/ping") == 0) {
              strcat(reply, token);
            } else {
              strcat(reply, "Invalid GET request");
            }
            
          } else if(strncmp(request, "POST", 3) == 0) {
            char main_reply [MSG_SIZE];
            if(strcmp(token, "/change") == 0) {
              isCelsius = !isCelsius;
              write(arduino, "c\0", 1);
              strcpy(main_reply, "Temp metric changed");
            } else if (strcmp(token, "/disarm") == 0) {
              write(arduino, "d\0", 1);
              strcpy(main_reply, "Alarm disarmed");
            } else if (strcmp(token, "/arm") == 0) {
              write(arduino, "a\0", 1);
              strcpy(main_reply, "Arming alarm");
            } else if (strcmp(token, "/toggle") == 0) {
              write(arduino, "t\0", 1);
              strcpy(main_reply, "Display toggled");
            } else {
              strcpy(main_reply, "Invalid POST request");
            }
            if (message_received()) {
              strcat(reply, main_reply);
              puts("reply to pebble");
              puts(main_reply);
              puts("end reply");
            } else {
              strcat(reply, "Lost connection with Arduino");
            }
          } else {
            strcat(reply,"Only able to handle GET and POST requests");
          }

        } else {
          strcat(reply, "no message received from pebble");

        }
        puts("REPLY:");
        puts(reply);

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
